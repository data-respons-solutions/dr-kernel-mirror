// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2023 NXP.
 */

#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/scmi_protocol.h>
#include <linux/scmi_imx_protocol.h>
#include <linux/reboot.h>

struct scmi_imx_bbm {
        struct scmi_protocol_handle *ph;
        const struct scmi_imx_bbm_proto_ops *ops;
};

static int bbm_poweroff(struct sys_off_data *data)
{
    struct scmi_imx_bbm *bbnsm = data->cb_data;
    struct scmi_protocol_handle *ph = bbnsm->ph;
    u32 state = 0;

    printk("iMX95: notify System Manager to power off\n");

    bbnsm->ops->shutdown_set(ph, &state);
    return NOTIFY_DONE;
}

static int scmi_imx_bbm_probe(struct scmi_device *sdev)
{
        const struct scmi_handle *handle = sdev->handle;
        struct device *dev = &sdev->dev;
        struct scmi_protocol_handle *ph;
        struct scmi_imx_bbm *bbnsm;
        int ret;

        if (!handle)
                return -ENODEV;

        bbnsm = devm_kzalloc(dev, sizeof(struct scmi_imx_bbm), GFP_KERNEL);
        if (!bbnsm)
                return -ENOMEM;

        bbnsm->ops = handle->devm_protocol_get(sdev, SCMI_PROTOCOL_IMX_BBM, &ph);
        if (IS_ERR(bbnsm->ops))
                return PTR_ERR(bbnsm->ops);

        bbnsm->ph = ph;

        dev_set_drvdata(dev, bbnsm);

        ret = devm_register_power_off_handler(dev, bbm_poweroff, bbnsm);
    if (ret) {
        dev_err(dev, "poweroff register failed: %d\n", ret);
        return ret;
    }
        return 0;
}

static const struct scmi_device_id scmi_id_table[] = {
        { SCMI_PROTOCOL_IMX_BBM, "imx-bbm" },
        { },
};
MODULE_DEVICE_TABLE(scmi, scmi_id_table);

static struct scmi_driver scmi_imx_bbm_driver = {
        .name = "scmi-imx-bbm",
        .probe = scmi_imx_bbm_probe,
        .id_table = scmi_id_table,
};
module_scmi_driver(scmi_imx_bbm_driver);

MODULE_AUTHOR("Peng Fan <peng.fan@nxp.com>");
MODULE_DESCRIPTION("IMX SM BBM driver");
MODULE_LICENSE("GPL");
