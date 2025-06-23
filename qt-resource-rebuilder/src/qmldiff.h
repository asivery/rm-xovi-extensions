#pragma once
extern int qmldiff_build_change_files(const char *rootDir);
extern char *qmldiff_process_file(const char *fileName, char *contents, size_t contentsLength);
extern char qmldiff_is_modified(const char *fileName);
extern char qmldiff_add_external_diff(const char *contents, const char *id);
extern void qmldiff_start_saving_thread();
extern void qmldiff_load_rules(const char *rules);
extern void qmldiff_set_version(const char *version);
