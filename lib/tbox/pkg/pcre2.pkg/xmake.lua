-- add pcre2 package
option("pcre2")

    -- show menu
    set_showmenu(true)

    -- set category
    set_category("package")

    -- set description
    set_description("The pcre2 package")
    
    -- add defines to config.h if checking ok
    add_defines_h_if_ok("$(prefix)_PACKAGE_HAVE_PCRE2")

    -- add defines for checking
    add_defines("PCRE2_CODE_UNIT_WIDTH=8")

    -- add links for checking
    add_links("pcre2-8")

    -- add link directories
    add_linkdirs("lib/$(plat)/$(arch)")

    -- add c includes for checking
    add_cincludes("pcre2/pcre2.h")

    -- add include directories
    add_includedirs("inc/$(plat)", "inc")

    -- add c functions
    add_cfuncs("pcre2_compile")
