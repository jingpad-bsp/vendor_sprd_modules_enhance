#include "DpuEnhanceCore.h"
#include "EnhanceModule.h"

extern DpuEnhanceCore *dpu_enhance;

int abc_cmd_hander(int argc, char *argv[]) {
    int mode;
    unsigned long int ambient;

    if (!strcmp(argv[2], "disable")) {
        abc_set_mode(ABC_MODE_DISABLE);
        return 0;
    } else if (!strcmp(argv[2], "mode=normal"))
        mode = ABC_MODE_NORMAL;
    else if (!strcmp(argv[2], "mode=lowpower"))
        mode = ABC_MODE_LOW_POWER;
    else {
        printf("error: invalid cmd \"%s\"\n", argv[2]);
        return -EINVAL;
    }

    if (!strcmp(argv[3], "index=ui"))
        mode |= ABC_MODE_UI;
    else if (!strcmp(argv[3], "index=game"))
        mode |= ABC_MODE_GAME;
    else if (!strcmp(argv[3], "index=video"))
        mode |= ABC_MODE_VIDEO;
    else if (!strcmp(argv[3], "index=image"))
        mode |= ABC_MODE_IMAGE;
    else if (!strcmp(argv[3], "index=camera"))
        mode |= ABC_MODE_CAMERA;
    else if (!strcmp(argv[3], "index=full_frame_video"))
        mode |= (ABC_MODE_VIDEO | ABC_MODE_FULL_FRAME);
    else {
        printf("error: invalid cmd \"%s\"\n", argv[3]);
        return -EINVAL;
    }

    abc_set_mode(mode);

    if (argc > 4) {
        if (!strncmp(argv[4], "ambient=", 8)) {
            ambient = strtoul(argv[4] + 8, NULL, 0);
            abc_set_value(ambient);
        } else {
            printf("error: invalid cmd \"%s\"\n", argv[4]);
            return -EINVAL;
        }
    }

    return 0;
}

int bld_cmd_hander(int argc, char *argv[]) {
    unsigned long int mode;

    if (!strcmp(argv[2], "disable"))
        bld_set_mode(BLD_MODE_DISABLE);
    else if (!strcmp(argv[2], "mode=default"))
        bld_set_mode(BLD_MODE_DEFAULT);
    else if (!strcmp(argv[2], "mode=middle"))
        bld_set_mode(BLD_MODE_MIDDLE);
    else if (!strcmp(argv[2], "mode=high"))
        bld_set_mode(BLD_MODE_HIGH);
    else if (!strncmp(argv[2], "mode=", 5)) {
        mode = strtoul(argv[2] + 5, NULL, 0);
        bld_set_mode(mode);
    } else {
        printf("error: invalid cmd \"%s\"\n", argv[2]);
        return -EINVAL;
    }

    return 0;
}

int cms_cmd_hander(int argc, char *argv[]) {
    unsigned long int temperature;

    if (!strcmp(argv[2], "disable"))
        cms_set_mode(CMS_MODE_DISABLE);
    else if (!strcmp(argv[2], "mode=auto"))
        cms_set_mode(CMS_MODE_AUTO);
    else if (!strcmp(argv[2], "mode=enhance"))
        cms_set_mode(CMS_MODE_ENHANCE);
    else if (!strcmp(argv[2], "mode=standard"))
        cms_set_mode(CMS_MODE_STANDARD);
    else if (!strncmp(argv[2], "temperature=", 12)) {
        cms_set_mode(CMS_MODE_AUTO);

        if (!strcmp(argv[2] + 12, "nature"))
            cms_set_value(CMS_TEMP_NATURE);
        else if (!strcmp(argv[2] + 12, "warm"))
            cms_set_value(CMS_TEMP_WARM);
        else if (!strcmp(argv[2] + 12, "cold"))
            cms_set_value(CMS_TEMP_COLD);
        else {
            temperature = strtoul(argv[2] + 12, NULL, 0);
            cms_set_value(temperature);
        }
    } else {
        printf("error: invalid cmd \"%s\"\n", argv[2]);
        return -EINVAL;
    }

    return 0;
}

int gamma_cmd_hander(int argc, char *argv[]) {

    if (!strcmp(argv[2], "disable"))
        gamma_set_mode(GAMMA_MODE_DISABLE);
    else if (!strcmp(argv[2], "mode=default"))
        gamma_set_mode(GAMMA_MODE_DEFAULT);
    else if (!strcmp(argv[2], "mode=standard"))
        gamma_set_mode(GAMMA_MODE_STANDARD);
    else {
        printf("error: invalid cmd \"%s\"\n", argv[2]);
        return -EINVAL;
    }

    return 0;
}

int read_cmd_hander(int argc, char *argv[]) {

    if (!strcmp(argv[1], "slp"))
        dpu_enhance->slpGetValue();
    else if (!strcmp(argv[1], "cm"))
        dpu_enhance->cmGetValue();
    else if (!strcmp(argv[1], "hsv"))
        dpu_enhance->hsvGetValue();
    else if (!strcmp(argv[1], "gamma"))
        dpu_enhance->gammaGetValue();
    else if (!strcmp(argv[1], "epf"))
        dpu_enhance->epfGetValue();
    else {
        printf("error: invalid module \"%s\"\n", argv[1]);
        return -EINVAL;
    }

    return 0;
}

void show_message(void) {
    printf("\nUsage:\n");
    printf("    enhance_test [module] [param]=[value]\n");
    printf("    enhance_test [module] read\n");
    printf("    enhance_test [module] disable\n\n");

    printf("For ABC:\n");
    printf("    module:   abc\n");
    printf("    param:    mode                      | index\n");
    printf("    value:    disable, normal, lowpower | ui game video full_frame_video image camera\n");
    printf("    param:    ambient\n");
    printf("    value:    see the ambient value in abc.xml\n");
    printf("    Example:\n");
    printf("              enhance_test abc mode=normal index=ui\n");
    printf("              enhance_test abc mode=normal index=ui ambient=7\n");
    printf("              enhance_test abc disable\n");
    printf("              enhance_test slp read\n\n");

    printf("For BLD:\n");
    printf("    module:   bld\n");
    printf("    param:    mode\n");
    printf("    value:    disable, default, middle, high, 0~64\n");
    printf("    Example:\n");
    printf("              enhance_test bld mode=default\n");
    printf("              enhance_test bld mode=35\n");
    printf("              enhance_test bld disable\n");
    printf("              enhance_test hsv read\n");
    printf("              enhance_test cm read\n\n");

    printf("For CMS:\n");
    printf("    module:   cms\n");
    printf("    param:    mode                             | temperature\n");
    printf("    value:    disable, auto, enhance, standard | cold, warm, see the rgb value in cms.xml\n");
    printf("    Example:\n");
    printf("              enhance_test cms mode=standard\n");
    printf("              enhance_test cms temperature=cold\n");
    printf("              enhance_test cms temperature=1\n");
    printf("              enhance_test cms disable\n");
    printf("              enhance_test hsv read\n");
    printf("              enhance_test cm read\n\n");

    printf("For Gamma:\n");
    printf("    module:   gamma\n");
    printf("    param:    mode\n");
    printf("    value:    disable, default, standard\n");
    printf("    Example:\n");
    printf("              enhance_test gamma mode=default\n");
    printf("              enhance_test gamma disable\n");
    printf("              enhance_test gamma read\n\n");
}

int main(int argc, char *argv[]) {
    int ret;
    static char dpu_version[20];

    pthread_once(&g_init, init_g_lock);

    if ((argc == 1) || (argc > 5)) {
        show_message();
        return 0;
    }

    ret = get_dpu_version(dpu_version, sizeof(dpu_version));
    if (ret <= 0)
        return -1;

    ret = create_dpu_enhance_core(dpu_version);
    if (ret)
        return -1;

    if (!strcmp(argv[2], "read")) {
        ret = read_cmd_hander(argc, argv);
    } else if (!strcmp(argv[1], "abc")) {
        get_enhance_param_from_xml(PQ_ID_SLP, dpu_version);
        ret = abc_cmd_hander(argc, argv);
    } else if (!strcmp(argv[1], "bld")) {
        get_enhance_param_from_xml(PQ_ID_BLP, dpu_version);
        ret = bld_cmd_hander(argc, argv);
    } else if (!strcmp(argv[1], "cms")) {
        get_enhance_param_from_xml(PQ_ID_CMS, dpu_version);
        ret = cms_cmd_hander(argc, argv);
    } else if (!strcmp(argv[1], "gamma")) {
        get_enhance_param_from_xml(PQ_ID_GAM, dpu_version);
        ret = gamma_cmd_hander(argc, argv);
    } else {
        printf("error: invalid module \"%s\"\n", argv[1]);
        ret = -EINVAL;
    }

    if (dpu_enhance) {
        delete dpu_enhance;
        dpu_enhance = NULL;
    }

    return ret;
}

