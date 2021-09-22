idf_component_register(SRCS "esp_idf.c"
                            "../../extensions/test//src/test.c"
                            "../../../src/xcmd.c"
                            "../../../src/xcmd_default_keys.c"
                            "../../../src/xcmd_default_cmds.c"
                    INCLUDE_DIRS "." 
                            "../../../inc/"
                            "../../extensions/test/inc/")
