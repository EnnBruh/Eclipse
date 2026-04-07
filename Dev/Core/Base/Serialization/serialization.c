#include "Base/base_internal.h"

ENNDEF_PRIVATE void datafile_create(DataFile* df) {
        DEBUG_TRACE();
        DEBUG_ASSERT(df != NULL);
        memset(df, 0, (sizeof (DataFile)));
        DEBUG_UNTRACE();
}

static void datafile_destroy_node(DataFileNode* node) {
        DEBUG_TRACE();
        DEBUG_ASSERT(node != NULL);
        if (node -> type == ENN_LIST) {
                for (i32 i = node -> data.children.start; i < node -> data.children.end; ++i)
                        datafile_destroy_node(node -> data.children.data[i]);
                vector_destroy(node -> data.children);
        }
        free(node);
        DEBUG_UNTRACE();
}

ENNDEF_PRIVATE void datafile_destroy(DataFile* df) {
        DEBUG_TRACE();
        DEBUG_ASSERT(df != NULL);
        datafile_destroy_node(df -> root);
        DEBUG_UNTRACE();
}

static DataFileNode* datafile_parse_node(DataFileNode* parent, char** cursor) {
        DEBUG_TRACE();
        DEBUG_ASSERT(cursor != NULL);

        DataFileNode* node = calloc(1, (sizeof (DataFileNode)));
        node -> parent = parent;

        while (strchr(ENN_DATAFILE_WHITESPACE, (**cursor))) ++(*cursor);

        char* start;

        if ((**cursor) == ENN_DATAFILE_STRING_IDENTIFIER) {
                start = ++(*cursor);
                while ((**cursor) != ENN_DATAFILE_STRING_IDENTIFIER) ++(*cursor);
                ++(*cursor);

                memcpy(node -> key, start, ((*cursor) - start - 1) * (sizeof (char)));
        } else DEBUG_LOG_WARN("Expected string identifier '%c' in %.*s", ENN_DATAFILE_STRING_IDENTIFIER, 10, *cursor);

        while (strchr(ENN_DATAFILE_WHITESPACE, (**cursor))) ++(*cursor);

        if ((**cursor) == ENN_DATAFILE_OPERATOR_ASSIGN) ++(*cursor);
        else DEBUG_LOG_WARN("Expected assign operator '%c' in %.*s", ENN_DATAFILE_OPERATOR_ASSIGN, 10, *cursor);

        while (strchr(ENN_DATAFILE_WHITESPACE, (**cursor))) ++(*cursor);

        switch (**cursor) {
                case ENN_DATAFILE_STRING_IDENTIFIER:
                        {
                                start = ++(*cursor);
                                while ((**cursor) != ENN_DATAFILE_STRING_IDENTIFIER) ++(*cursor);
                                ++(*cursor);

                                node -> type = ENN_STRING;
                                memcpy(node -> data.string_val, start, ((*cursor) - start- 1) * (sizeof (char)));
                                goto end_of_func;
                        }
                case ENN_DATAFILE_OPERATOR_LIST_BEGIN:
                        {

                                ++(*cursor);
                                node -> type = ENN_LIST;
                                DataFileNode* child = datafile_parse_node(node, cursor);
                                vector_push_back(node -> data.children, child);
                                do {
                                        while (strchr(ENN_DATAFILE_WHITESPACE, (**cursor))) ++(*cursor);

                                        switch (**cursor) {
                                                case ENN_DATAFILE_OPERATOR_LIST_CONTINUE:
                                                        {
                                                                ++(*cursor);
                                                                child = datafile_parse_node(node, cursor);
                                                                vector_push_back(node -> data.children, child);
                                                                break;
                                                        }
                                                case ENN_DATAFILE_OPERATOR_LIST_END:
                                                        {
                                                                ++(*cursor);
                                                                goto end_of_func;
                                                                break;
                                                        }
                                        }
                                } while (1);
                                goto end_of_func;
                        }
        }

        if (strchr(ENN_DATAFILE_NUMBER_SYMBOLS, **cursor)) {
                start = (*cursor);

                bool is_real = false;
                while (strchr(ENN_DATAFILE_NUMBER_SYMBOLS, (**cursor))) {
                        if ((**cursor) == '.') is_real = true;
                        ++(*cursor);
                }

                if (is_real) {
                        node -> type = ENN_REAL;
                        node -> data.real_val = strtof(start, cursor);
                } else {
                        node -> type = ENN_INT;
                        node -> data.int_val = strtol(start, cursor, 10);
                }
        } else DEBUG_LOG_WARN("Unknown data type for key '%s' in %.*s", node -> key, 10, *cursor);

end_of_func:
        DEBUG_UNTRACE();
        return node;
}


static i32 datafile_write_node_precalc(DataFileNode* node, i32 depth) {
        DEBUG_TRACE();
        i32 to_be_written = 0;
        switch (node -> type) {
                case ENN_INT:
                        to_be_written += snprintf(NULL, 0, "%*c%s%c %c %" PRIi32, ENN_DATAFILE_LIST_ITEM_PADDING * depth + 1, ENN_DATAFILE_STRING_IDENTIFIER, node -> key, ENN_DATAFILE_STRING_IDENTIFIER, ENN_DATAFILE_OPERATOR_ASSIGN, node -> data.int_val);
                        break;
                case ENN_REAL:
                        to_be_written += snprintf(NULL, 0, "%*c%s%c %c %f", ENN_DATAFILE_LIST_ITEM_PADDING * depth + 1, ENN_DATAFILE_STRING_IDENTIFIER, node -> key, ENN_DATAFILE_STRING_IDENTIFIER, ENN_DATAFILE_OPERATOR_ASSIGN, node -> data.real_val);
                        break;
                case ENN_STRING:
                        to_be_written += snprintf(NULL, 0, "%*c%s%c %c %c%s%c", ENN_DATAFILE_LIST_ITEM_PADDING * depth + 1, ENN_DATAFILE_STRING_IDENTIFIER, node -> key, ENN_DATAFILE_STRING_IDENTIFIER, ENN_DATAFILE_OPERATOR_ASSIGN, ENN_DATAFILE_STRING_IDENTIFIER, node -> data.string_val, ENN_DATAFILE_STRING_IDENTIFIER);
                        break;
                case ENN_LIST:
                        {
                                to_be_written += snprintf(NULL, 0, "%*c%s%c %c %c\n", ENN_DATAFILE_LIST_ITEM_PADDING * depth + 1, ENN_DATAFILE_STRING_IDENTIFIER, node -> key, ENN_DATAFILE_STRING_IDENTIFIER, ENN_DATAFILE_OPERATOR_ASSIGN, ENN_DATAFILE_OPERATOR_LIST_BEGIN);
                                for (i32 i = node -> data.children.start; i < node -> data.children.end; ++i) {
                                        to_be_written += datafile_write_node_precalc(node -> data.children.data[i], depth + 1);
                                        if (i + 1 < node -> data.children.end) to_be_written += snprintf(NULL, 0, "%c\n", ENN_DATAFILE_OPERATOR_LIST_CONTINUE);
                                        else to_be_written += snprintf(NULL, 0, "\n");
                                }
                                to_be_written += snprintf(NULL, 0, "%*c", ENN_DATAFILE_LIST_ITEM_PADDING * depth + 1, ENN_DATAFILE_OPERATOR_LIST_END);
                                break;
                        }
        }
        DEBUG_UNTRACE();
        return to_be_written;
}


static void datafile_write_node(DataFileNode* node, i32 depth, char** cursor) {
        DEBUG_TRACE();
        switch (node -> type) {
                case ENN_INT:
                        *cursor += sprintf(*cursor, "%*c%s%c %c %" PRIi32, ENN_DATAFILE_LIST_ITEM_PADDING * depth + 1, ENN_DATAFILE_STRING_IDENTIFIER, node -> key, ENN_DATAFILE_STRING_IDENTIFIER, ENN_DATAFILE_OPERATOR_ASSIGN, node -> data.int_val);
                        break;
                case ENN_REAL:
                        *cursor += sprintf(*cursor, "%*c%s%c %c %f", ENN_DATAFILE_LIST_ITEM_PADDING * depth + 1, ENN_DATAFILE_STRING_IDENTIFIER, node -> key, ENN_DATAFILE_STRING_IDENTIFIER, ENN_DATAFILE_OPERATOR_ASSIGN, node -> data.real_val);
                        break;
                case ENN_STRING:
                        *cursor += sprintf(*cursor, "%*c%s%c %c %c%s%c", ENN_DATAFILE_LIST_ITEM_PADDING * depth + 1, ENN_DATAFILE_STRING_IDENTIFIER, node -> key, ENN_DATAFILE_STRING_IDENTIFIER, ENN_DATAFILE_OPERATOR_ASSIGN, ENN_DATAFILE_STRING_IDENTIFIER, node -> data.string_val, ENN_DATAFILE_STRING_IDENTIFIER);
                        break;
                case ENN_LIST:
                        {
                                *cursor += sprintf(*cursor, "%*c%s%c %c %c\n", ENN_DATAFILE_LIST_ITEM_PADDING * depth + 1, ENN_DATAFILE_STRING_IDENTIFIER, node -> key, ENN_DATAFILE_STRING_IDENTIFIER, ENN_DATAFILE_OPERATOR_ASSIGN, ENN_DATAFILE_OPERATOR_LIST_BEGIN);
                                for (i32 i = node -> data.children.start; i < node -> data.children.end; ++i) {
                                        datafile_write_node(node -> data.children.data[i], depth + 1, cursor);
                                        if (i + 1 < node -> data.children.end) *cursor += sprintf(*cursor, "%c\n", ENN_DATAFILE_OPERATOR_LIST_CONTINUE);
                                        else *cursor += sprintf(*cursor, "\n");
                                }
                                *cursor += sprintf(*cursor, "%*c", ENN_DATAFILE_LIST_ITEM_PADDING * depth + 1, ENN_DATAFILE_OPERATOR_LIST_END);
                                break;
                        }
        }
        DEBUG_UNTRACE();
}



ENNDEF_PUBLIC DataFileNode* datafile_parse_keypath_find_or_create_node(DataFile* df, char* keypath) {
        DEBUG_TRACE();
        DEBUG_ASSERT(df != NULL);
        DEBUG_ASSERT(keypath != NULL);

        DataFileNode* root = df -> root;

        i32 i;
        bool found = false;

        char* cursor = keypath;
        char* token = cursor;
        while ((*cursor) && (*cursor) != ENN_DATAFILE_KEYPATH_SEPARATOR) ++cursor;
        if (root == NULL) {
                DataFileNode* new_node = calloc(1, (sizeof (DataFileNode)));
                DEBUG_ASSERT(new_node != NULL);

                memcpy(new_node -> key, token, (cursor - token) * (sizeof (char)));
                root = new_node;
                df -> root = root;
        } else if (memcmp(token, root -> key, (cursor - token) * (sizeof (char))) != 0) {
                DataFileNode* new_node = calloc(1, (sizeof (DataFileNode)));
                DEBUG_ASSERT(new_node != NULL);

                memcpy(new_node -> key, token, (cursor - token) * (sizeof (char)));
                new_node -> parent = root;
                root -> type = ENN_LIST;
                vector_push_back(root -> data.children, new_node);
                root = new_node;
        }

        if ((*cursor)) ++cursor;

        while (*cursor) {
                char* token = cursor;
                while ((*cursor) && (*cursor) != ENN_DATAFILE_KEYPATH_SEPARATOR) ++cursor;

                found = false;
                for (i = root -> data.children.start; i < root -> data.children.end; ++i)
                        if (memcmp(token, root -> data.children.data[i] -> key, (cursor - token) * (sizeof (char))) == 0) {
                                found = true;
                                root = root -> data.children.data[i];
                                break;
                        }
                if (!found) {
                        DataFileNode* new_node = calloc(1, (sizeof (DataFileNode)));
                        DEBUG_ASSERT(new_node != NULL);

                        memcpy(new_node -> key, token, (cursor - token) * (sizeof (char)));

                        new_node -> parent = root;
                        root -> type = ENN_LIST;
                        vector_push_back(root -> data.children, new_node);
                        root = new_node;
                }

                if ((*cursor)) ++cursor;
        }

        DEBUG_UNTRACE();
        return root;
}

void datafile_put_i32(DataFile* df, char* keypath, i32 val) {
        DEBUG_TRACE();
        DataFileNode* node = datafile_parse_keypath_find_or_create_node(df, keypath);

        switch (node -> type) {
                case ENN_LIST:
                        {
                                for (i32 i = node -> data.children.start; i < node -> data.children.end; ++i)
                                        datafile_destroy_node(node -> data.children.data[i]);
                                break;
                        }
                default: break;
        }

        node -> type = ENN_INT;
        node -> data.int_val = val;
        DEBUG_UNTRACE();
}

void datafile_put_f32(DataFile* df, char* keypath, f32 val) {
        DEBUG_TRACE();
        DataFileNode* node = datafile_parse_keypath_find_or_create_node(df, keypath);

        switch (node -> type) {
                case ENN_LIST:
                        {
                                for (i32 i = node -> data.children.start; i < node -> data.children.end; ++i)
                                        datafile_destroy_node(node -> data.children.data[i]);
                                break;
                        }
                default: break;
        }

        node -> type = ENN_REAL;
        node -> data.real_val = val;
        DEBUG_UNTRACE();
}

void datafile_put_cstring(DataFile* df, char* keypath, const char* val) {
        DEBUG_TRACE();
        DataFileNode* node = datafile_parse_keypath_find_or_create_node(df, keypath);

        switch (node -> type) {
                case ENN_LIST:
                        {
                                for (i32 i = node -> data.children.start; i < node -> data.children.end; ++i)
                                        datafile_destroy_node(node -> data.children.data[i]);
                                break;
                        }
                default: break;
        }

        node -> type = ENN_STRING;

        i32 size = 0;
        while (val[size++]);
        memcpy(node -> data.string_val, val, size * (sizeof (char)));
        DEBUG_UNTRACE();
}

ENNDEF_PUBLIC DataFileNode* datafile_parse_keypath_find_node(DataFile* df, char* keypath) {
        DEBUG_TRACE();
        DEBUG_ASSERT(df != NULL);
        DEBUG_ASSERT(keypath != NULL);


        DataFileNode* root = df -> root;
        i32 i;
        bool found = false;

        char* cursor = keypath;
        char* token = cursor;
        while ((*cursor) && (*cursor) != ENN_DATAFILE_KEYPATH_SEPARATOR) ++cursor;

        if (root == NULL) return NULL;
        if (memcmp(token, root -> key, (cursor - token - 1) * (sizeof (char))) == 0);
        else return NULL;

        if ((*cursor)) ++cursor;

        while (*cursor) {
                char* token = cursor;
                while ((*cursor) && (*cursor) != ENN_DATAFILE_KEYPATH_SEPARATOR) ++cursor;

                found = false;
                for (i = root -> data.children.start; i < root -> data.children.end; ++i)
                        if (memcmp(token, root -> data.children.data[i] -> key, (cursor - token - 1) * (sizeof (char))) == 0) {
                                found = true;
                                root = root -> data.children.data[i];
                                break;
                        }

                if (!found) return NULL;

                if ((*cursor)) ++cursor;
        }


        DEBUG_UNTRACE();
        return root;
}

i32 datafile_get_i32(DataFile* df, char* keypath) {
        DEBUG_TRACE();
        DataFileNode* node = datafile_parse_keypath_find_node(df, keypath);
        DEBUG_ASSERT(node != NULL);
        DEBUG_UNTRACE();
        if (node -> type == ENN_INT) return node -> data.int_val;
        return 0;
}

f32 datafile_get_f32(DataFile* df, char* keypath) {
        DEBUG_TRACE();
        DataFileNode* node = datafile_parse_keypath_find_node(df, keypath);
        DEBUG_ASSERT(node != NULL);
        DEBUG_UNTRACE();
        if (node -> type == ENN_REAL) return node -> data.real_val;
        return 0.0;
}

char* datafile_get_cstring_absolute(DataFile* df, char* keypath) {
        DEBUG_TRACE();
        DataFileNode* node = datafile_parse_keypath_find_node(df, keypath);
        DEBUG_ASSERT(node != NULL);
        DEBUG_UNTRACE();
        if (node -> type == ENN_STRING) return node -> data.string_val;
        return NULL;
}

void datafile_read(DataFile* df, const char* filepath) {
        DEBUG_TRACE();
        DEBUG_ASSERT(df != NULL);
        DEBUG_ASSERT(filepath != NULL);

        DEBUG_ASSERT(strcmp(filepath + strlen(filepath) - (sizeof ENN_DATAFILE_FILE_EXTENSION), ENN_DATAFILE_FILE_EXTENSION) == 0, "Serialization library only accepts files with the extesion %s", ENN_DATAFILE_FILE_EXTENSION);

        char* file_data;
        file_read_cstring(filepath, &file_data);

        char* copy = file_data;
        df -> root = datafile_parse_node(NULL, &copy);
        free(file_data);
        DEBUG_UNTRACE();
}

void datafile_read_abstract(DataFile* df, const char* buff) {
	DEBUG_TRACE();
	DEBUG_ASSERT(df != NULL);
	DEBUG_ASSERT(buff!= NULL);

	char* copy = (char*)buff;
	df -> root = datafile_parse_node(NULL, &copy);
	DEBUG_UNTRACE();
}

i32 datafile_write_precalc(DataFile* df) {
	DEBUG_TRACE();
	DEBUG_ASSERT(df != NULL);

	DEBUG_UNTRACE();
	return datafile_write_node_precalc(df -> root, 0);
}

void datafile_write_abstract(DataFile* df, const char* buff) {
	DEBUG_TRACE();
	DEBUG_ASSERT(df != NULL);
	DEBUG_ASSERT(buff != NULL);

	char* copy = (char*)buff;
	datafile_write_node(df -> root, 0, &copy);
	DEBUG_UNTRACE();
}

void datafile_write(DataFile* df, const char* filepath) {
        DEBUG_TRACE();
        DEBUG_ASSERT(df != NULL);
        DEBUG_ASSERT(filepath != NULL);

        i32 reserve_size = datafile_write_node_precalc(df -> root, 0);
        DEBUG_ASSERT(reserve_size > 0);

        char* file_data = calloc(reserve_size + 1, (sizeof (char)));
        char* copy = file_data;
        datafile_write_node(df -> root, 0, &copy);

        file_write_cstring(filepath, file_data, reserve_size + 1);
        free(file_data);
        DEBUG_UNTRACE();
}
