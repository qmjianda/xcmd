idf_component_register(SRCS "esp_idf.c"
                            "test_cmds.c"
                            "../../../src/xcmd.c"
                            "../../../src/xcmd_default_keys.c"
                            "../../../src/xcmd_default_cmds.c"
                    INCLUDE_DIRS "." 
                            "../../../inc/")
