/*
 * Virtio Console and Generic Serial Port Devices
 *
 * Copyright Red Hat, Inc. 2009, 2010
 *
 * Authors:
 *  Amit Shah <amit.shah@redhat.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#include "sysemu/char.h"
#include "qemu/error-report.h"
#include "trace.h"
#include "hw/virtio/virtio-serial.h"

#define TYPE_VIRTIO_CONSOLE "virtconsole"
#define VIRTIO_CONSOLE(obj) \
    OBJECT_CHECK(VirtConsole, (obj), TYPE_VIRTIO_CONSOLE)
#define VIRTIO_CONSOLE_GET_CLASS(obj) \
    OBJECT_GET_CLASS(VirtConsoleClass, (obj), TYPE_VIRTIO_CONSOLE)
#define VIRTIO_CONSOLE_CLASS(cls) \
    OBJECT_CLASS_CHECK(VirtConsoleClass, (cls), TYPE_VIRTIO_CONSOLE)

typedef struct VirtConsole {
    VirtIOSerialPort parent_obj;

    CharDriverState *chr;
    guint watch;
} VirtConsole;

typedef struct VirtConsoleClass {
    VirtIOSerialPortClass parent_class;

    DeviceRealize parent_realize;
    DeviceUnrealize parent_unrealize;
} VirtConsoleClass;

/*
 * Callback function that's called from chardevs when backend becomes
 * writable.
 */
static gboolean chr_write_unblocked(GIOChannel *chan, GIOCondition cond,
                                    void *opaque)
{
    VirtConsole *vcon = opaque;

    vcon->watch = 0;
    virtio_serial_throttle_port(VIRTIO_SERIAL_PORT(vcon), false);
    return FALSE;
}

/* Callback function that's called when the guest sends us data */
static ssize_t flush_buf(VirtIOSerialPort *port,
                         const uint8_t *buf, ssize_t len)
{
    VirtConsole *vcon = VIRTIO_CONSOLE(port);
    ssize_t ret;

    if (!vcon->chr) {
        /* If there's no backend, we can just say we consumed all data. */
        return len;
    }

    ret = qemu_chr_fe_write(vcon->chr, buf, len);
    trace_virtio_console_flush_buf(port->id, len, ret);

    if (ret < len) {
        VirtIOSerialPortClass *k = VIRTIO_SERIAL_PORT_GET_CLASS(port);

        /*
         * Ideally we'd get a better error code than just -1, but
         * that's what the chardev interface gives us right now.  If
         * we had a finer-grained message, like -EPIPE, we could close
         * this connection.
         */
        if (ret < 0)
            ret = 0;
        if (!k->is_console) {
            virtio_serial_throttle_port(port, true);
            if (!vcon->watch) {
                vcon->watch = qemu_chr_fe_add_watch(vcon->chr, G_IO_OUT,
                                                    chr_write_unblocked, vcon);
            }
        }
    }
    return ret;
}

/* Callback function that's called when the guest opens/closes the port */
static void set_guest_connected(VirtIOSerialPort *port, int guest_connected)
{
    VirtConsole *vcon = VIRTIO_CONSOLE(port);

    if (!vcon->chr) {
        return;
    }
    qemu_chr_fe_set_open(vcon->chr, guest_connected);
}

/* Readiness of the guest to accept data on a port */
static int chr_can_read(void *opaque)
{
    VirtConsole *vcon = opaque;

    return virtio_serial_guest_ready(VIRTIO_SERIAL_PORT(vcon));
}

/* Send data from a char device over to the guest */
static void chr_read(void *opaque, const uint8_t *buf, int size)
{
    VirtConsole *vcon = opaque;
    VirtIOSerialPort *port = VIRTIO_SERIAL_PORT(vcon);

    trace_virtio_console_chr_read(port->id, size);
    virtio_serial_write(port, buf, size);
}

static void chr_event(void *opaque, int event)
{
    VirtConsole *vcon = opaque;
    VirtIOSerialPort *port = VIRTIO_SERIAL_PORT(vcon);

    trace_virtio_console_chr_event(port->id, event);
    switch (event) {
    case CHR_EVENT_OPENED:
        virtio_serial_open(port);
        break;
    case CHR_EVENT_CLOSED:
        if (vcon->watch) {
            g_source_remove(vcon->watch);
            vcon->watch = 0;
        }
        virtio_serial_close(port);
        break;
    }
}

static void virtconsole_realize(DeviceState *dev, Error **errp)
{
    VirtIOSerialPort *port = VIRTIO_SERIAL_PORT(dev);
    VirtConsole *vcon = VIRTIO_CONSOLE(dev);
    VirtIOSerialPortClass *k = VIRTIO_SERIAL_PORT_GET_CLASS(dev);
    VirtConsoleClass *vcc = VIRTIO_CONSOLE_GET_CLASS(dev);

    if (port->id == 0 && !k->is_console) {
        error_setg(errp, "Port number 0 on virtio-serial devices reserved "
                   "for virtconsole devices for backward compatibility.");
        return;
    }

    if (vcon->chr) {
        vcon->chr->explicit_fe_open = 1;
        qemu_chr_add_handlers(vcon->chr, chr_can_read, chr_read, chr_event,
                              vcon);
    }

    vcc->parent_realize(dev, errp);
}

static void virtconsole_unrealize(DeviceState *dev, Error **errp)
{
    VirtConsole *vcon = VIRTIO_CONSOLE(dev);
    VirtConsoleClass *vcc = VIRTIO_CONSOLE_GET_CLASS(dev);
    Error *err = NULL;

    vcc->parent_unrealize(dev, &err);
    if (err != NULL) {
        error_propagate(errp, err);
        return;
    }

    if (vcon->watch) {
        g_source_remove(vcon->watch);
    }
}

static Property virtconsole_properties[] = {
    DEFINE_PROP_CHR("chardev", VirtConsole, chr),
    DEFINE_PROP_END_OF_LIST(),
};

static void virtconsole_class_init(ObjectClass *oc, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(oc);
    VirtIOSerialPortClass *k = VIRTIO_SERIAL_PORT_CLASS(oc);
    VirtConsoleClass *vcc = VIRTIO_CONSOLE_CLASS(oc);

    vcc->parent_realize = dc->realize;
    dc->realize = virtconsole_realize;

    vcc->parent_unrealize = dc->unrealize;
    dc->unrealize = virtconsole_unrealize;

    k->is_console = true;
    k->have_data = flush_buf;
    k->set_guest_connected = set_guest_connected;
    dc->props = virtconsole_properties;
}

static const TypeInfo virtconsole_info = {
    .name          = TYPE_VIRTIO_CONSOLE,
    .parent        = TYPE_VIRTIO_SERIAL_PORT,
    .instance_size = sizeof(VirtConsole),
    .class_init    = virtconsole_class_init,
    .class_size    = sizeof(VirtConsoleClass),
};

static Property virtserialport_properties[] = {
    DEFINE_PROP_CHR("chardev", VirtConsole, chr),
    DEFINE_PROP_END_OF_LIST(),
};

static void virtserialport_class_init(ObjectClass *oc, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(oc);
    VirtIOSerialPortClass *k = VIRTIO_SERIAL_PORT_CLASS(oc);
    VirtConsoleClass *vcc = VIRTIO_CONSOLE_CLASS(oc);

    vcc->parent_realize = dc->realize;
    dc->realize = virtconsole_realize;

    vcc->parent_unrealize = dc->unrealize;
    dc->unrealize = virtconsole_unrealize;

    k->have_data = flush_buf;
    k->set_guest_connected = set_guest_connected;
    dc->props = virtserialport_properties;
}

static const TypeInfo virtserialport_info = {
    .name          = "virtserialport",
    .parent        = TYPE_VIRTIO_SERIAL_PORT,
    .instance_size = sizeof(VirtConsole),
    .class_init    = virtserialport_class_init,
};

static void virtconsole_register_types(void)
{
    type_register_static(&virtconsole_info);
    type_register_static(&virtserialport_info);
}

type_init(virtconsole_register_types)
