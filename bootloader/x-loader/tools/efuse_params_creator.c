/*
 *  Copyright (C) 2016 Ingenic Semiconductor Co.,Ltd
 *
 *  X1000 series bootloader for u-boot/rtos/linux
 *
 *  Wang Qiuwei <qiuwei.wang@ingenic.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <common.h>
#include <string.h>
#include <assert.h>

#ifdef printf
#undef printf
#endif
#include <stdio.h>


struct efuse_cfg {
    uint32_t rd_adj;
    uint32_t wr_adj;
    uint32_t rd_strobe;
    uint32_t wr_strobe;
};

static int efuse_adjust_cfg(struct efuse_cfg *cfg) {
    int i;
    uint32_t val, ns;
    uint32_t h2clk;
    uint32_t pll_rate = CONFIG_DDR_SEL_PLL == APLL ? CONFIG_APLL_FREQ : CONFIG_MPLL_FREQ;

    h2clk = pll_rate / CONFIG_AHB_CLK_DIV * 1000000;
    ns = 1000000000 / h2clk;

    for(i = 0; i < 0x4; i++)
        if(((i + 1) * ns) > 2)
            break;

    assert((i < 0x4));
    cfg->rd_adj = cfg->wr_adj = i;

    for(i = 0; i < 0x8; i++)
        if(((cfg->rd_adj + i + 3) * ns) > 15)
            break;

    assert((i < 0x8));
    cfg->rd_strobe = i;

    /*
     * X-loader efuse driver not support to write efuse,
     * so, don't need to calculate wr_adj and wr_strobe.
     */
#if 0
    cfg->wr_adj = cfg->rd_adj;

    for(i = 0; i < 0x1f; i += 100) {
        val = (cfg->wr_adj + i + 916) * ns;
        if( val > 4 * 1000 && val < 6 *1000)
            break;
    }

    assert((i < 0x1f));
    cfg->wr_strobe = i;
#endif
    return 0;
}

static void params_print(struct efuse_cfg *cfg) {
    printf("/*\n");
    printf(" * DO NOT MODIFY.\n");
    printf(" *\n");
    printf(" * This file was generated by efuse_params_creator\n");
    printf(" *\n");
    printf(" */\n");
    printf("\n");

    printf("#ifndef EFUSE_REG_VALUES_H\n");
    printf("#define EFUSE_REG_VALUES_H\n\n");

    printf("\n");
    printf("#define EFUCFG_RD_ADJ       0x%x\n", cfg->rd_adj);
    printf("#define EFUCFG_WR_ADJ       0x%x\n", cfg->wr_adj);
    printf("#define EFUCFG_RD_STROBE    0x%x\n", cfg->rd_strobe);
    printf("#define EFUCFG_WR_STROBE    0x%x\n", cfg->wr_strobe);
    printf("\n");
    printf("#endif /* EFUSE_REG_VALUES_H */\n");
}

int main(void) {
    int ret = 0;
    struct efuse_cfg cfg;

    memset(&cfg, 0, sizeof(struct efuse_cfg));

    ret = efuse_adjust_cfg(&cfg);
    if(ret < 0)
        return -1;

    params_print(&cfg);

    return 0;
}
