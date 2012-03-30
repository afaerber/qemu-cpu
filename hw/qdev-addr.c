#include "qemu/object.h"
#include "qdev.h"
#include "qdev-addr.h"
#include "targphys.h"

/* --- target physical address --- */

static int parse_taddr(Object *obj, Property *prop, const char *str)
{
    target_phys_addr_t *ptr = object_get_prop_ptr(obj, prop);

    *ptr = strtoull(str, NULL, 16);
    return 0;
}

static int print_taddr(Object *obj, Property *prop, char *dest, size_t len)
{
    target_phys_addr_t *ptr = object_get_prop_ptr(obj, prop);
    return snprintf(dest, len, "0x" TARGET_FMT_plx, *ptr);
}

static void get_taddr(Object *obj, Visitor *v, void *opaque,
                      const char *name, Error **errp)
{
    Property *prop = opaque;
    target_phys_addr_t *ptr = object_get_prop_ptr(obj, prop);
    int64_t value;

    value = *ptr;
    visit_type_int64(v, &value, name, errp);
}

static void set_taddr(Object *obj, Visitor *v, void *opaque,
                      const char *name, Error **errp)
{
    Property *prop = opaque;
    target_phys_addr_t *ptr = object_get_prop_ptr(obj, prop);
    Error *local_err = NULL;
    int64_t value;

    if (object_is_realized(obj)) {
        error_set(errp, QERR_PERMISSION_DENIED);
        return;
    }

    visit_type_int64(v, &value, name, &local_err);
    if (local_err) {
        error_propagate(errp, local_err);
        return;
    }
    if ((uint64_t)value <= (uint64_t) ~(target_phys_addr_t)0) {
        *ptr = value;
    } else {
        error_set(errp, QERR_INVALID_PARAMETER_VALUE,
                  name, "target_phys_addr_t");
    }
}


PropertyInfo qdev_prop_taddr = {
    .name  = "taddr",
    .parse = parse_taddr,
    .print = print_taddr,
    .get   = get_taddr,
    .set   = set_taddr,
};

void qdev_prop_set_taddr(DeviceState *dev, const char *name, target_phys_addr_t value)
{
    Error *errp = NULL;
    object_property_set_int(OBJECT(dev), value, name, &errp);
    assert(!errp);

}
