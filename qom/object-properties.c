#include "qemu/object.h"
#include "qapi/qapi-visit-core.h"

void *object_get_prop_ptr(Object *obj, Property *prop)
{
    void *ptr = obj;
    ptr += prop->offset;
    return ptr;
}

static uint32_t get_prop_mask(Property *prop)
{
    assert(prop->info == &qdev_prop_bit);
    return 0x1 << prop->bitnr;
}

static void bit_prop_set(Object *obj, Property *props, bool val)
{
    uint32_t *p = object_get_prop_ptr(obj, props);
    uint32_t mask = get_prop_mask(props);
    if (val)
        *p |= mask;
    else
        *p &= ~mask;
}

/* Bit */

static int print_bit(Object *obj, Property *prop, char *dest, size_t len)
{
    uint32_t *p = object_get_prop_ptr(obj, prop);
    return snprintf(dest, len, (*p & get_prop_mask(prop)) ? "on" : "off");
}

static void get_bit(Object *obj, Visitor *v, void *opaque,
                    const char *name, Error **errp)
{
    Property *prop = opaque;
    uint32_t *p = object_get_prop_ptr(obj, prop);
    bool value = (*p & get_prop_mask(prop)) != 0;

    visit_type_bool(v, &value, name, errp);
}

static void set_bit(Object *obj, Visitor *v, void *opaque,
                    const char *name, Error **errp)
{
    Property *prop = opaque;
    Error *local_err = NULL;
    bool value;

    if (object_is_realized(obj)) {
        error_set(errp, QERR_PERMISSION_DENIED);
        return;
    }

    visit_type_bool(v, &value, name, &local_err);
    if (local_err) {
        error_propagate(errp, local_err);
        return;
    }
    bit_prop_set(obj, prop, value);
}

PropertyInfo qdev_prop_bit = {
    .name  = "boolean",
    .legacy_name  = "on/off",
    .print = print_bit,
    .get   = get_bit,
    .set   = set_bit,
};

/* --- 8bit integer --- */

static void get_uint8(Object *obj, Visitor *v, void *opaque,
                      const char *name, Error **errp)
{
    Property *prop = opaque;
    uint8_t *ptr = object_get_prop_ptr(obj, prop);

    visit_type_uint8(v, ptr, name, errp);
}

static void set_uint8(Object *obj, Visitor *v, void *opaque,
                      const char *name, Error **errp)
{
    Property *prop = opaque;
    uint8_t *ptr = object_get_prop_ptr(obj, prop);

    if (object_is_realized(obj)) {
        error_set(errp, QERR_PERMISSION_DENIED);
        return;
    }

    visit_type_uint8(v, ptr, name, errp);
}

PropertyInfo qdev_prop_uint8 = {
    .name  = "uint8",
    .get   = get_uint8,
    .set   = set_uint8,
};

/* --- 8bit hex value --- */

static int parse_hex8(Object *obj, Property *prop, const char *str)
{
    uint8_t *ptr = object_get_prop_ptr(obj, prop);
    char *end;

    if (str[0] != '0' || str[1] != 'x') {
        return -EINVAL;
    }

    *ptr = strtoul(str, &end, 16);
    if ((*end != '\0') || (end == str)) {
        return -EINVAL;
    }

    return 0;
}

static int print_hex8(Object *obj, Property *prop, char *dest, size_t len)
{
    uint8_t *ptr = object_get_prop_ptr(obj, prop);
    return snprintf(dest, len, "0x%" PRIx8, *ptr);
}

PropertyInfo qdev_prop_hex8 = {
    .name  = "uint8",
    .legacy_name  = "hex8",
    .parse = parse_hex8,
    .print = print_hex8,
    .get   = get_uint8,
    .set   = set_uint8,
};

/* --- 16bit integer --- */

static void get_uint16(Object *obj, Visitor *v, void *opaque,
                       const char *name, Error **errp)
{
    Property *prop = opaque;
    uint16_t *ptr = object_get_prop_ptr(obj, prop);

    visit_type_uint16(v, ptr, name, errp);
}

static void set_uint16(Object *obj, Visitor *v, void *opaque,
                       const char *name, Error **errp)
{
    Property *prop = opaque;
    uint16_t *ptr = object_get_prop_ptr(obj, prop);

    if (object_is_realized(obj)) {
        error_set(errp, QERR_PERMISSION_DENIED);
        return;
    }

    visit_type_uint16(v, ptr, name, errp);
}

PropertyInfo qdev_prop_uint16 = {
    .name  = "uint16",
    .get   = get_uint16,
    .set   = set_uint16,
};

/* --- 32bit integer --- */

static void get_uint32(Object *obj, Visitor *v, void *opaque,
                       const char *name, Error **errp)
{
    Property *prop = opaque;
    uint32_t *ptr = object_get_prop_ptr(obj, prop);

    visit_type_uint32(v, ptr, name, errp);
}

static void set_uint32(Object *obj, Visitor *v, void *opaque,
                       const char *name, Error **errp)
{
    Property *prop = opaque;
    uint32_t *ptr = object_get_prop_ptr(obj, prop);

    if (object_is_realized(obj)) {
        error_set(errp, QERR_PERMISSION_DENIED);
        return;
    }

    visit_type_uint32(v, ptr, name, errp);
}

static void get_int32(Object *obj, Visitor *v, void *opaque,
                      const char *name, Error **errp)
{
    Property *prop = opaque;
    int32_t *ptr = object_get_prop_ptr(obj, prop);

    visit_type_int32(v, ptr, name, errp);
}

static void set_int32(Object *obj, Visitor *v, void *opaque,
                      const char *name, Error **errp)
{
    Property *prop = opaque;
    int32_t *ptr = object_get_prop_ptr(obj, prop);

    if (object_is_realized(obj)) {
        error_set(errp, QERR_PERMISSION_DENIED);
        return;
    }

    visit_type_int32(v, ptr, name, errp);
}

PropertyInfo qdev_prop_uint32 = {
    .name  = "uint32",
    .get   = get_uint32,
    .set   = set_uint32,
};

PropertyInfo qdev_prop_int32 = {
    .name  = "int32",
    .get   = get_int32,
    .set   = set_int32,
};

/* --- 32bit hex value --- */

static int parse_hex32(Object *obj, Property *prop, const char *str)
{
    uint32_t *ptr = object_get_prop_ptr(obj, prop);
    char *end;

    if (str[0] != '0' || str[1] != 'x') {
        return -EINVAL;
    }

    *ptr = strtoul(str, &end, 16);
    if ((*end != '\0') || (end == str)) {
        return -EINVAL;
    }

    return 0;
}

static int print_hex32(Object *obj, Property *prop, char *dest, size_t len)
{
    uint32_t *ptr = object_get_prop_ptr(obj, prop);
    return snprintf(dest, len, "0x%" PRIx32, *ptr);
}

PropertyInfo qdev_prop_hex32 = {
    .name  = "uint32",
    .legacy_name  = "hex32",
    .parse = parse_hex32,
    .print = print_hex32,
    .get   = get_uint32,
    .set   = set_uint32,
};

/* --- 64bit integer --- */

static void get_uint64(Object *obj, Visitor *v, void *opaque,
                       const char *name, Error **errp)
{
    Property *prop = opaque;
    uint64_t *ptr = object_get_prop_ptr(obj, prop);

    visit_type_uint64(v, ptr, name, errp);
}

static void set_uint64(Object *obj, Visitor *v, void *opaque,
                       const char *name, Error **errp)
{
    Property *prop = opaque;
    uint64_t *ptr = object_get_prop_ptr(obj, prop);

    if (object_is_realized(obj)) {
        error_set(errp, QERR_PERMISSION_DENIED);
        return;
    }

    visit_type_uint64(v, ptr, name, errp);
}

PropertyInfo qdev_prop_uint64 = {
    .name  = "uint64",
    .get   = get_uint64,
    .set   = set_uint64,
};

/* --- 64bit hex value --- */

static int parse_hex64(Object *obj, Property *prop, const char *str)
{
    uint64_t *ptr = object_get_prop_ptr(obj, prop);
    char *end;

    if (str[0] != '0' || str[1] != 'x') {
        return -EINVAL;
    }

    *ptr = strtoull(str, &end, 16);
    if ((*end != '\0') || (end == str)) {
        return -EINVAL;
    }

    return 0;
}

static int print_hex64(Object *obj, Property *prop, char *dest, size_t len)
{
    uint64_t *ptr = object_get_prop_ptr(obj, prop);
    return snprintf(dest, len, "0x%" PRIx64, *ptr);
}

PropertyInfo qdev_prop_hex64 = {
    .name  = "uint64",
    .legacy_name  = "hex64",
    .parse = parse_hex64,
    .print = print_hex64,
    .get   = get_uint64,
    .set   = set_uint64,
};

/* --- string --- */

static void release_string(Object *obj, const char *name, void *opaque)
{
    Property *prop = opaque;
    g_free(*(char **)object_get_prop_ptr(obj, prop));
}

static int print_string(Object *obj, Property *prop, char *dest, size_t len)
{
    char **ptr = object_get_prop_ptr(obj, prop);
    if (!*ptr)
        return snprintf(dest, len, "<null>");
    return snprintf(dest, len, "\"%s\"", *ptr);
}

static void get_string(Object *obj, Visitor *v, void *opaque,
                       const char *name, Error **errp)
{
    Property *prop = opaque;
    char **ptr = object_get_prop_ptr(obj, prop);

    if (!*ptr) {
        char *str = (char *)"";
        visit_type_str(v, &str, name, errp);
    } else {
        visit_type_str(v, ptr, name, errp);
    }
}

static void set_string(Object *obj, Visitor *v, void *opaque,
                       const char *name, Error **errp)
{
    Property *prop = opaque;
    char **ptr = object_get_prop_ptr(obj, prop);
    Error *local_err = NULL;
    char *str;

    if (object_is_realized(obj)) {
        error_set(errp, QERR_PERMISSION_DENIED);
        return;
    }

    visit_type_str(v, &str, name, &local_err);
    if (local_err) {
        error_propagate(errp, local_err);
        return;
    }
    if (*ptr) {
        g_free(*ptr);
    }
    *ptr = str;
}

PropertyInfo qdev_prop_string = {
    .name  = "string",
    .print = print_string,
    .release = release_string,
    .get   = get_string,
    .set   = set_string,
};


/* --- enums --- */

void property_get_enum(Object *obj, Visitor *v, void *opaque,
                       const char *name, Error **errp)
{
    Property *prop = opaque;
    int *ptr = object_get_prop_ptr(obj, prop);

    visit_type_enum(v, ptr, prop->info->enum_table,
                    prop->info->name, prop->name, errp);
}

void property_set_enum(Object *obj, Visitor *v, void *opaque,
                       const char *name, Error **errp)
{
    Property *prop = opaque;
    int *ptr = object_get_prop_ptr(obj, prop);

    if (object_is_realized(obj)) {
        error_set(errp, QERR_PERMISSION_DENIED);
        return;
    }

    visit_type_enum(v, ptr, prop->info->enum_table,
                    prop->info->name, prop->name, errp);
}


/**
 * @object_property_add_static - add a @Property to a device.
 *
 * Static properties access data in a struct.  The actual type of the
 * property and the field depends on the property type.
 */
void object_property_add_static(Object *obj, Property *prop,
                                Error **errp)
{
    Error *local_err = NULL;

    /*
     * TODO qdev_prop_ptr does not have getters or setters.  It must
     * go now that it can be replaced with links.  The test should be
     * removed along with it: all static properties are read/write.
     */
    if (!prop->info->get && !prop->info->set) {
        return;
    }

    object_property_add(obj, prop->name, prop->info->name,
                        prop->info->get, prop->info->set,
                        prop->info->release,
                        prop, &local_err);

    if (local_err) {
        error_propagate(errp, local_err);
        return;
    }
    if (prop->qtype == QTYPE_NONE) {
        return;
    }

    if (prop->qtype == QTYPE_QBOOL) {
        object_property_set_bool(obj, prop->defval, prop->name, &local_err);
    } else if (prop->info->enum_table) {
        object_property_set_str(obj, prop->info->enum_table[prop->defval],
                                prop->name, &local_err);
    } else if (prop->qtype == QTYPE_QINT) {
        object_property_set_int(obj, prop->defval, prop->name, &local_err);
    }
    assert_no_error(local_err);
}
