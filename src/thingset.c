/*
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* The main implementation file of the thingset library. */
#define THINGSET_MAIN 1

#include "thingset_priv.h"

#include <stdio.h>
#include <stdlib.h>

static void _check_id_duplicates(const struct ts_data_object *data, size_t num)
{
    for (unsigned int i = 0; i < num; i++) {
        for (unsigned int j = i + 1; j < num; j++) {
            if (data[i].id == data[j].id) {
                LOG_ERR("ThingSet error: Duplicate data object ID 0x%X.\n", data[i].id);
            }
        }
    }
}

int ts_init(struct ts_context *ts, struct ts_data_object *data, size_t num)
{
    _check_id_duplicates(data, num);

    ts->data_objects = data;
    ts->num_objects = num;
    ts->_auth_flags = TS_USR_MASK;

    return 0;
}

#ifdef CONFIG_THINGSET_ITERABLE_SECTIONS

/*
 * ThingSet data objects are generated in each module using Zephyr's iterable section feature.
 *
 * Below pointers are normally used in the STRUCT_SECTION_FOREACH macro, but we want to use
 * a pointer to the array of data objects directly, so we extract the memory locations manually.
 */
extern struct ts_data_object _ts_data_object_list_start[];
extern struct ts_data_object _ts_data_object_list_end[];

int ts_init_global(struct ts_context *ts)
{
    /* duplicates are checked at compile-time */

    ts->data_objects = _ts_data_object_list_start;
    ts->num_objects = _ts_data_object_list_end - _ts_data_object_list_start;
    ts->_auth_flags = TS_USR_MASK;

    return 0;
}

#endif

int ts_process(struct ts_context *ts, const uint8_t *request, size_t request_len,
               uint8_t *response, size_t response_size)
{
    // check if proper request was set before asking for a response
    if (request == NULL || request_len < 1) {
        return 0;
    }

    // assign private variables
    ts->req = request;
    ts->req_len = request_len;
    ts->resp = response;
    ts->resp_size = response_size;

    if (ts->req[0] < 0x20) {
        // binary mode request
        return ts_bin_process(ts);
    }
    else if (ts->req[0] == '?' || ts->req[0] == '=' || ts->req[0] == '+'
               || ts->req[0] == '-' || ts->req[0] == '!') {
        // text mode request
        return ts_txt_process(ts);
    }
    else {
        // not a thingset command --> ignore and set response to empty string
        response[0] = '\0';
        return 0;
    }
}

void ts_set_authentication(struct ts_context *ts, uint8_t flags)
{
    ts->_auth_flags = flags;
}

void ts_set_update_callback(struct ts_context *ts, const uint16_t subsets, void (*update_cb)(void))
{
    ts->_update_subsets = subsets;
    ts->update_cb = update_cb;
}

struct ts_data_object *ts_get_object_by_name(struct ts_context *ts, const char *name,
                                             size_t len, int32_t parent)
{
    for (unsigned int i = 0; i < ts->num_objects; i++) {
        if (parent != -1 && ts->data_objects[i].parent != parent) {
            continue;
        }
        else if (strncmp(ts->data_objects[i].name, name, len) == 0
            // without length check foo and fooBar would be recognized as equal
            && strlen(ts->data_objects[i].name) == len)
        {
            return &(ts->data_objects[i]);
        }
    }
    return NULL;
}

struct ts_data_object *ts_get_object_by_id(struct ts_context *ts, ts_object_id_t id)
{
    for (unsigned int i = 0; i < ts->num_objects; i++) {
        if (ts->data_objects[i].id == id) {
            return &(ts->data_objects[i]);
        }
    }
    return NULL;
}

struct ts_data_object *ts_get_endpoint_by_path(struct ts_context *ts, const char *path, size_t len,
    int *index)
{
    struct ts_data_object *object = NULL;
    const char *start = path;
    const char *end;
    uint16_t parent = 0;

    // maximum depth of 10 assumed
    for (int i = 0; i < 10; i++) {
        end = strchr(start, '/');
        if (end == NULL || end >= path + len) {
            // we are at the end of the path
            if (object != NULL && object->type == TS_T_RECORDS &&
                start[0] >= '0' && start[0] <= '9')
            {
                // numeric ID, only valid to select index in an array of records
                if (index != NULL) {
                    *index = strtoul(start, NULL, 0);
                }
                return object;
            }
            else {
                return ts_get_object_by_name(ts, start, path + len - start, parent);
            }
        }
        else if (end == path + len - 1) {
            // path ends with slash
            return ts_get_object_by_name(ts, start, end - start, parent);
        }
        else {
            // go further down the path
            object = ts_get_object_by_name(ts, start, end - start, parent);
            if (object) {
                parent = object->id;
                start = end + 1;
            }
            else {
                return NULL;
            }
        }
    }
    return NULL;
}

struct ts_data_object *ts_get_object_by_path(struct ts_context *ts, const char *path, size_t len)
{
    return ts_get_endpoint_by_path(ts, path, len, NULL);
}

int ts_get_path(struct ts_context *ts, char *buf, size_t size, const struct ts_data_object *obj)
{
    int pos = 0;
    if (obj->parent == 0) {
        pos = snprintf(buf, size, "%s", obj->name);
    }
    else {
        struct ts_data_object *parent_obj = ts_get_object_by_id(ts, obj->parent);
        if (parent_obj != NULL) {
            pos = snprintf(buf, size, "%s/%s", parent_obj->name, obj->name);
        }
        else {
            return 0;
        }
    }

    if (pos < size) {
        return pos;
    }
    else {
        // path did not fit into the buffer
        return 0;
    }
}
