/*
 * Copyright (c) 2013-2016 Qlogic Corporation
 * All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  and ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * File: ql_os.c
 * Author : David C Somayajulu, Qlogic Corporation, Aliso Viejo, CA 92656.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");


#include "ql_os.h"
#include "ql_hw.h"
#include "ql_def.h"
#include "ql_inline.h"
#include "ql_ver.h"
#include "ql_glbl.h"
#include "ql_dbg.h"
#include <sys/smp.h>

/*
 * Some PCI Configuration Space Related Defines
 */

#ifndef PCI_VENDOR_QLOGIC
#define PCI_VENDOR_QLOGIC	0x1077
#endif

#ifndef PCI_PRODUCT_QLOGIC_ISP8030
#define PCI_PRODUCT_QLOGIC_ISP8030	0x8030
#endif

#define PCI_QLOGIC_ISP8030 \
	((PCI_PRODUCT_QLOGIC_ISP8030 << 16) | PCI_VENDOR_QLOGIC)

/*
 * static functions
 */
static int qla_alloc_parent_dma_tag(qla_host_t *ha);
static void qla_free_parent_dma_tag(qla_host_t *ha);
static int qla_alloc_xmt_bufs(qla_host_t *ha);
static void qla_free_xmt_bufs(qla_host_t *ha);
static int qla_alloc_rcv_bufs(qla_host_t *ha);
static void qla_free_rcv_bufs(qla_host_t *ha);
static void qla_clear_tx_buf(qla_host_t *ha, qla_tx_buf_t *txb);

static void qla_init_ifnet(device_t dev, qla_host_t *ha);
static int qla_sysctl_get_stats(SYSCTL_HANDLER_ARGS);
static int qla_sysctl_get_link_status(SYSCTL_HANDLER_ARGS);
static void qla_release(qla_host_t *ha);
static void qla_dmamap_callback(void *arg, bus_dma_segment_t *segs, int nsegs,
		int error);
static void qla_stop(qla_host_t *ha);
static int qla_send(qla_host_t *ha, struct mbuf **m_headp);
static void qla_tx_done(void *context, int pending);
static void qla_get_peer(qla_host_t *ha);
static void qla_error_recovery(void *context, int pending);
static void qla_async_event(void *context, int pending);

/*
 * Hooks to the Operating Systems
 */
static int qla_pci_probe (device_t);
static int qla_pci_attach (device_t);
static int qla_pci_detach (device_t);

static void qla_init(void *arg);
static int qla_ioctl(struct ifnet *ifp, u_long cmd, caddr_t data);
static int qla_media_change(struct ifnet *ifp);
static void qla_media_status(struct ifnet *ifp, struct ifmediareq *ifmr);
static void qla_start(struct ifnet *ifp);

static device_method_t qla_pci_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe, qla_pci_probe),
	DEVMETHOD(device_attach, qla_pci_attach),
	DEVMETHOD(device_detach, qla_pci_detach),
	{ 0, 0 }
};

static driver_t qla_pci_driver = {
	"ql", qla_pci_methods, sizeof (qla_host_t),
};

static devclass_t qla83xx_devclass;

DRIVER_MODULE(qla83xx, pci, qla_pci_driver, qla83xx_devclass, 0, 0);

MODULE_DEPEND(qla83xx, pci, 1, 1, 1);
MODULE_DEPEND(qla83xx, ether, 1, 1, 1);

MALLOC_DEFINE(M_QLA83XXBUF, "qla83xxbuf", "Buffers for qla83xx driver");

#define QL_STD_REPLENISH_THRES		0
#define QL_JUMBO_REPLENISH_THRES	32


static char dev_str[64];
static char ver_str[64];

/*
 * Name:	qla_pci_probe
 * Function:	Validate the PCI device to be a QLA80XX device
 */
static int
qla_pci_probe(device_t dev)
{
        switch ((pci_get_device(dev) << 16) | (pci_get_vendor(dev))) {
        case PCI_QLOGIC_ISP8030:
		snprintf(dev_str, sizeof(dev_str), "%s v%d.%d.%d",
			"Qlogic ISP 83xx PCI CNA Adapter-Ethernet Function",
			QLA_VERSION_MAJOR, QLA_VERSION_MINOR,
			QLA_VERSION_BUILD);
		snprintf(ver_str, sizeof(ver_str), "v%d.%d.%d",
			QLA_VERSION_MAJOR, QLA_VERSION_MINOR,
			QLA_VERSION_BUILD);
                device_set_desc(dev, dev_str);
                break;
        default:
                return (ENXIO);
        }

        if (bootverbose)
                printf("%s: %s\n ", __func__, dev_str);

        return (BUS_PROBE_DEFAULT);
}

static void
qla_add_sysctls(qla_host_t *ha)
{
        device_t dev = ha->pci_dev;

	SYSCTL_ADD_STRING(device_get_sysctl_ctx(dev),
		SYSCTL_CHILDREN(device_get_sysctl_tree(dev)),
		OID_AUTO, "version", CTLFLAG_RD,
		ver_str, 0, "Driver Version");

        SYSCTL_ADD_PROC(device_get_sysctl_ctx(dev),
                SYSCTL_CHILDREN(device_get_sysctl_tree(dev)),
                OID_AUTO, "stats", CTLTYPE_INT | CTLFLAG_RW,
                (void *)ha, 0,
                qla_sysctl_get_stats, "I", "Statistics");

        SYSCTL_ADD_STRING(device_get_sysctl_ctx(dev),
                SYSCTL_CHILDREN(device_get_sysctl_tree(dev)),
                OID_AUTO, "fw_version", CTLFLAG_RD,
                ha->fw_ver_str, 0, "firmware version");

        SYSCTL_ADD_PROC(device_get_sysctl_ctx(dev),
                SYSCTL_CHILDREN(device_get_sysctl_tree(dev)),
                OID_AUTO, "link_status", CTLTYPE_INT | CTLFLAG_RW,
                (void *)ha, 0,
                qla_sysctl_get_link_status, "I", "Link Status");

	ha->dbg_level = 0;
        SYSCTL_ADD_UINT(device_get_sysctl_ctx(dev),
                SYSCTL_CHILDREN(device_get_sysctl_tree(dev)),
                OID_AUTO, "debug", CTLFLAG_RW,
                &ha->dbg_level, ha->dbg_level, "Debug Level");

	ha->std_replenish = QL_STD_REPLENISH_THRES;
        SYSCTL_ADD_UINT(device_get_sysctl_ctx(dev),
                SYSCTL_CHILDREN(device_get_sysctl_tree(dev)),
                OID_AUTO, "std_replenish", CTLFLAG_RW,
                &ha->std_replenish, ha->std_replenish,
                "Threshold for Replenishing Standard Frames");

        SYSCTL_ADD_QUAD(device_get_sysctl_ctx(dev),
                SYSCTL_CHILDREN(device_get_sysctl_tree(dev)),
                OID_AUTO, "ipv4_lro",
                CTLFLAG_RD, &ha->ipv4_lro,
                "number of ipv4 lro completions");

        SYSCTL_ADD_QUAD(device_get_sysctl_ctx(dev),
                SYSCTL_CHILDREN(device_get_sysctl_tree(dev)),
                OID_AUTO, "ipv6_lro",
                CTLFLAG_RD, &ha->ipv6_lro,
                "number of ipv6 lro completions");

	SYSCTL_ADD_QUAD(device_get_sysctl_ctx(dev),
		SYSCTL_CHILDREN(device_get_sysctl_tree(dev)),
		OID_AUTO, "tx_tso_frames",
		CTLFLAG_RD, &ha->tx_tso_frames,
		"number of Tx TSO Frames");

	SYSCTL_ADD_QUAD(device_get_sysctl_ctx(dev),
                SYSCTL_CHILDREN(device_get_sysctl_tree(dev)),
		OID_AUTO, "hw_vlan_tx_frames",
		CTLFLAG_RD, &ha->hw_vlan_tx_frames,
		"number of Tx VLAN Frames");

        return;
}

static void
qla_watchdog(void *arg)
{
	qla_host_t *ha = arg;
	qla_hw_t *hw;
	struct ifnet *ifp;
	uint32_t i;
	qla_hw_tx_cntxt_t *hw_tx_cntxt;

	hw = &ha->hw;
	ifp = ha->ifp;

        if (ha->flags.qla_watchdog_exit) {
		ha->qla_watchdog_exited = 1;
		return;
	}
	ha->qla_watchdog_exited = 0;

	if (!ha->flags.qla_watchdog_pause) {
		if (ql_hw_check_health(ha) || ha->qla_initiate_recovery ||
			(ha->msg_from_peer == QL_PEER_MSG_RESET)) {
			ha->qla_watchdog_paused = 1;
			ha->flags.qla_watchdog_pause = 1;
			ha->qla_initiate_recovery = 0;
			ha->err_inject = 0;
			device_printf(ha->pci_dev,
				"%s: taskqueue_enqueue(err_task) \n", __func__);
			taskqueue_enqueue(ha->err_tq, &ha->err_task);
		} else if (ha->flags.qla_interface_up) {

                        if (ha->async_event) {
                                ha->async_event = 0;
                                taskqueue_enqueue(ha->async_event_tq,
                                        &ha->async_event_task);
                        }

			for (i = 0; i < ha->hw.num_tx_rings; i++) {
				hw_tx_cntxt = &hw->tx_cntxt[i];
				if (qla_le32_to_host(*(hw_tx_cntxt->tx_cons)) !=
					hw_tx_cntxt->txr_comp) {
					taskqueue_enqueue(ha->tx_tq,
						&ha->tx_task);
					break;
				}
			}

			if ((ifp->if_snd.ifq_head != NULL) && QL_RUNNING(ifp)) {
				taskqueue_enqueue(ha->tx_tq, &ha->tx_task);
			}
			ha->qla_watchdog_paused = 0;
		} else {
			ha->qla_watchdog_paused = 0;
		}
	} else {
		ha->qla_watchdog_paused = 1;
	}

	ha->watchdog_ticks = ha->watchdog_ticks++ % 1000;
	callout_reset(&ha->tx_callout, QLA_WATCHDOG_CALLOUT_TICKS,
		qla_watchdog, ha);
}

/*
 * Name:	qla_pci_attach
 * Function:	attaches the device to the operating system
 */
static int
qla_pci_attach(device_t dev)
{
	qla_host_t *ha = NULL;
	uint32_t rsrc_len;
	int i;
	uint32_t num_rcvq = 0;

        if ((ha = device_get_softc(dev)) == NULL) {
                device_printf(dev, "cannot get softc\n");
                return (ENOMEM);
        }

        memset(ha, 0, sizeof (qla_host_t));

        if (pci_get_device(dev) != PCI_PRODUCT_QLOGIC_ISP8030) {
                device_printf(dev, "device is not ISP8030\n");
                return (ENXIO);
	}

        ha->pci_func = pci_get_function(dev) & 0x1;

        ha->pci_dev = dev;

	pci_enable_busmaster(dev);

	ha->reg_rid = PCIR_BAR(0);
	ha->pci_reg = bus_alloc_resource_any(dev, SYS_RES_MEMORY, &ha->reg_rid,
				RF_ACTIVE);

        if (ha->pci_reg == NULL) {
                device_printf(dev, "unable to map any ports\n");
                goto qla_pci_attach_err;
        }

	rsrc_len = (uint32_t) bus_get_resource_count(dev, SYS_RES_MEMORY,
					ha->reg_rid);

	mtx_init(&ha->hw_lock, "qla83xx_hw_lock", MTX_NETWORK_LOCK, MTX_SPIN);

	mtx_init(&ha->tx_lock, "qla83xx_tx_lock", MTX_NETWORK_LOCK, MTX_DEF);

	qla_add_sysctls(ha);
	ql_hw_add_sysctls(ha);

	ha->flags.lock_init = 1;

	ha->reg_rid1 = PCIR_BAR(2);
	ha->pci_reg1 = bus_alloc_resource_any(dev, SYS_RES_MEMORY,
			&ha->reg_rid1, RF_ACTIVE);

	ha->msix_count = pci_msix_count(dev);

	if (ha->msix_count < (ha->hw.num_sds_rings + 1)) {
		device_printf(dev, "%s: msix_count[%d] not enough\n", __func__,
			ha->msix_count);
		goto qla_pci_attach_err;
	}

	QL_DPRINT2(ha, (dev, "%s: ha %p pci_func 0x%x rsrc_count 0x%08x"
		" msix_count 0x%x pci_reg %p\n", __func__, ha,
		ha->pci_func, rsrc_len, ha->msix_count, ha->pci_reg));

        /* initialize hardware */
        if (ql_init_hw(ha)) {
                device_printf(dev, "%s: ql_init_hw failed\n", __func__);
                goto qla_pci_attach_err;
        }

        device_printf(dev, "%s: firmware[%d.%d.%d.%d]\n", __func__,
                ha->fw_ver_major, ha->fw_ver_minor, ha->fw_ver_sub,
                ha->fw_ver_build);
        snprintf(ha->fw_ver_str, sizeof(ha->fw_ver_str), "%d.%d.%d.%d",
                        ha->fw_ver_major, ha->fw_ver_minor, ha->fw_ver_sub,
                        ha->fw_ver_build);

        if (qla_get_nic_partition(ha, NULL, &num_rcvq)) {
                device_printf(dev, "%s: qla_get_nic_partition failed\n",
                        __func__);
                goto qla_pci_attach_err;
        }
        device_printf(dev, "%s: ha %p pci_func 0x%x rsrc_count 0x%08x"
                " msix_count 0x%x pci_reg %p num_rcvq = %d\n", __func__, ha,
                ha->pci_func, rsrc_len, ha->msix_count, ha->pci_reg, num_rcvq);


#ifdef QL_ENABLE_ISCSI_TLV
        if ((ha->msix_count  < 64) || (num_rcvq != 32)) {
                ha->hw.num_sds_rings = 15;
                ha->hw.num_tx_rings = 32;
        }
#endif /* #ifdef QL_ENABLE_ISCSI_TLV */
	ha->hw.num_rds_rings = ha->hw.num_sds_rings;

	ha->msix_count = ha->hw.num_sds_rings + 1;

	if (pci_alloc_msix(dev, &ha->msix_count)) {
		device_printf(dev, "%s: pci_alloc_msi[%d] failed\n", __func__,
			ha->msix_count);
		ha->msix_count = 0;
		goto qla_pci_attach_err;
	}

	ha->mbx_irq_rid = 1;
	ha->mbx_irq = bus_alloc_resource_any(dev, SYS_RES_IRQ,
				&ha->mbx_irq_rid,
				(RF_ACTIVE | RF_SHAREABLE));
	if (ha->mbx_irq == NULL) {
		device_printf(dev, "could not allocate mbx interrupt\n");
		goto qla_pci_attach_err;
	}
	if (bus_setup_intr(dev, ha->mbx_irq, (INTR_TYPE_NET | INTR_MPSAFE),
		NULL, ql_mbx_isr, ha, &ha->mbx_handle)) {
		device_printf(dev, "could not setup mbx interrupt\n");
		goto qla_pci_attach_err;
	}

	for (i = 0; i < ha->hw.num_sds_rings; i++) {
		ha->irq_vec[i].sds_idx = i;
                ha->irq_vec[i].ha = ha;
                ha->irq_vec[i].irq_rid = 2 + i;

		ha->irq_vec[i].irq = bus_alloc_resource_any(dev, SYS_RES_IRQ,
				&ha->irq_vec[i].irq_rid,
				(RF_ACTIVE | RF_SHAREABLE));

		if (ha->irq_vec[i].irq == NULL) {
			device_printf(dev, "could not allocate interrupt\n");
			goto qla_pci_attach_err;
		}
		if (bus_setup_intr(dev, ha->irq_vec[i].irq,
			(INTR_TYPE_NET | INTR_MPSAFE),
			NULL, ql_isr, &ha->irq_vec[i],
			&ha->irq_vec[i].handle)) {
			device_printf(dev, "could not setup interrupt\n");
			goto qla_pci_attach_err;
		}
	}

	printf("%s: mp__ncpus %d sds %d rds %d msi-x %d\n", __func__, mp_ncpus,
		ha->hw.num_sds_rings, ha->hw.num_rds_rings, ha->msix_count);

	ql_read_mac_addr(ha);

	/* allocate parent dma tag */
	if (qla_alloc_parent_dma_tag(ha)) {
		device_printf(dev, "%s: qla_alloc_parent_dma_tag failed\n",
			__func__);
		goto qla_pci_attach_err;
	}

	/* alloc all dma buffers */
	if (ql_alloc_dma(ha)) {
		device_printf(dev, "%s: ql_alloc_dma failed\n", __func__);
		goto qla_pci_attach_err;
	}
	qla_get_peer(ha);

	if (ql_minidump_init(ha) != 0) {
		device_printf(dev, "%s: ql_minidump_init failed\n", __func__);
		goto qla_pci_attach_err;
	}
	/* create the o.s ethernet interface */
	qla_init_ifnet(dev, ha);

	ha->flags.qla_watchdog_active = 1;
	ha->flags.qla_watchdog_pause = 0;


	TASK_INIT(&ha->tx_task, 0, qla_tx_done, ha);
	ha->tx_tq = taskqueue_create("qla_txq", M_NOWAIT,
			taskqueue_thread_enqueue, &ha->tx_tq);
	taskqueue_start_threads(&ha->tx_tq, 1, PI_NET, "%s txq",
		device_get_nameunit(ha->pci_dev));
	
	callout_init(&ha->tx_callout, TRUE);
	ha->flags.qla_callout_init = 1;

	/* create ioctl device interface */
	if (ql_make_cdev(ha)) {
		device_printf(dev, "%s: ql_make_cdev failed\n", __func__);
		goto qla_pci_attach_err;
	}

	callout_reset(&ha->tx_callout, QLA_WATCHDOG_CALLOUT_TICKS,
		qla_watchdog, ha);

	TASK_INIT(&ha->err_task, 0, qla_error_recovery, ha);
	ha->err_tq = taskqueue_create("qla_errq", M_NOWAIT,
			taskqueue_thread_enqueue, &ha->err_tq);
	taskqueue_start_threads(&ha->err_tq, 1, PI_NET, "%s errq",
		device_get_nameunit(ha->pci_dev));

        TASK_INIT(&ha->async_event_task, 0, qla_async_event, ha);
        ha->async_event_tq = taskqueue_create("qla_asyncq", M_NOWAIT,
                        taskqueue_thread_enqueue, &ha->async_event_tq);
        taskqueue_start_threads(&ha->async_event_tq, 1, PI_NET, "%s asyncq",
                device_get_nameunit(ha->pci_dev));

	QL_DPRINT2(ha, (dev, "%s: exit 0\n", __func__));
        return (0);

qla_pci_attach_err:

	qla_release(ha);

	QL_DPRINT2(ha, (dev, "%s: exit ENXIO\n", __func__));
        return (ENXIO);
}

/*
 * Name:	qla_pci_detach
 * Function:	Unhooks the device from the operating system
 */
static int
qla_pci_detach(device_t dev)
{
	qla_host_t *ha = NULL;
	struct ifnet *ifp;

	QL_DPRINT2(ha, (dev, "%s: enter\n", __func__));

        if ((ha = device_get_softc(dev)) == NULL) {
                device_printf(dev, "cannot get softc\n");
                return (ENOMEM);
        }

	ifp = ha->ifp;

	(void)QLA_LOCK(ha, __func__, 0);
	qla_stop(ha);
	QLA_UNLOCK(ha, __func__);

	qla_release(ha);

	QL_DPRINT2(ha, (dev, "%s: exit\n", __func__));

        return (0);
}

/*
 * SYSCTL Related Callbacks
 */
static int
qla_sysctl_get_stats(SYSCTL_HANDLER_ARGS)
{
	int err, ret = 0;
	qla_host_t *ha;

	err = sysctl_handle_int(oidp, &ret, 0, req);

	if (err || !req->newptr)
		return (err);

	if (ret == 1) {
		ha = (qla_host_t *)arg1;
		ql_get_stats(ha);
	}
	return (err);
}
static int
qla_sysctl_get_link_status(SYSCTL_HANDLER_ARGS)
{
	int err, ret = 0;
	qla_host_t *ha;

	err = sysctl_handle_int(oidp, &ret, 0, req);

	if (err || !req->newptr)
		return (err);

	if (ret == 1) {
		ha = (qla_host_t *)arg1;
		ql_hw_link_status(ha);
	}
	return (err);
}

/*
 * Name:	qla_release
 * Function:	Releases the resources allocated for the device
 */
static void
qla_release(qla_host_t *ha)
{
	device_t dev;
	int i;

	dev = ha->pci_dev;

        if (ha->async_event_tq) {
                taskqueue_drain(ha->async_event_tq, &ha->async_event_task);
                taskqueue_free(ha->async_event_tq);
        }

	if (ha->err_tq) {
		taskqueue_drain(ha->err_tq, &ha->err_task);
		taskqueue_free(ha->err_tq);
	}

	if (ha->tx_tq) {
		taskqueue_drain(ha->tx_tq, &ha->tx_task);
		taskqueue_free(ha->tx_tq);
	}

	ql_del_cdev(ha);

	if (ha->flags.qla_watchdog_active) {
		ha->flags.qla_watchdog_exit = 1;

		while (ha->qla_watchdog_exited == 0)
			qla_mdelay(__func__, 1);
	}

	if (ha->flags.qla_callout_init)
		callout_stop(&ha->tx_callout);

	if (ha->ifp != NULL)
		ether_ifdetach(ha->ifp);

	ql_free_dma(ha); 
	qla_free_parent_dma_tag(ha);

	if (ha->mbx_handle)
		(void)bus_teardown_intr(dev, ha->mbx_irq, ha->mbx_handle);

	if (ha->mbx_irq)
		(void) bus_release_resource(dev, SYS_RES_IRQ, ha->mbx_irq_rid,
				ha->mbx_irq);

	for (i = 0; i < ha->hw.num_sds_rings; i++) {

		if (ha->irq_vec[i].handle) {
			(void)bus_teardown_intr(dev, ha->irq_vec[i].irq,
					ha->irq_vec[i].handle);
		}
			
		if (ha->irq_vec[i].irq) {
			(void)bus_release_resource(dev, SYS_RES_IRQ,
				ha->irq_vec[i].irq_rid,
				ha->irq_vec[i].irq);
		}
	}

	if (ha->msix_count)
		pci_release_msi(dev);

	if (ha->flags.lock_init) {
		mtx_destroy(&ha->tx_lock);
		mtx_destroy(&ha->hw_lock);
	}

        if (ha->pci_reg)
                (void) bus_release_resource(dev, SYS_RES_MEMORY, ha->reg_rid,
				ha->pci_reg);

        if (ha->pci_reg1)
                (void) bus_release_resource(dev, SYS_RES_MEMORY, ha->reg_rid1,
				ha->pci_reg1);
}

/*
 * DMA Related Functions
 */

static void
qla_dmamap_callback(void *arg, bus_dma_segment_t *segs, int nsegs, int error)
{
        *((bus_addr_t *)arg) = 0;

        if (error) {
                printf("%s: bus_dmamap_load failed (%d)\n", __func__, error);
                return;
	}

        *((bus_addr_t *)arg) = segs[0].ds_addr;

	return;
}

int
ql_alloc_dmabuf(qla_host_t *ha, qla_dma_t *dma_buf)
{
        int             ret = 0;
        device_t        dev;
        bus_addr_t      b_addr;

        dev = ha->pci_dev;

        QL_DPRINT2(ha, (dev, "%s: enter\n", __func__));

        ret = bus_dma_tag_create(
                        ha->parent_tag,/* parent */
                        dma_buf->alignment,
                        ((bus_size_t)(1ULL << 32)),/* boundary */
                        BUS_SPACE_MAXADDR,      /* lowaddr */
                        BUS_SPACE_MAXADDR,      /* highaddr */
                        NULL, NULL,             /* filter, filterarg */
                        dma_buf->size,          /* maxsize */
                        1,                      /* nsegments */
                        dma_buf->size,          /* maxsegsize */
                        0,                      /* flags */
                        NULL, NULL,             /* lockfunc, lockarg */
                        &dma_buf->dma_tag);

        if (ret) {
                device_printf(dev, "%s: could not create dma tag\n", __func__);
                goto ql_alloc_dmabuf_exit;
        }
        ret = bus_dmamem_alloc(dma_buf->dma_tag,
                        (void **)&dma_buf->dma_b,
                        (BUS_DMA_ZERO | BUS_DMA_COHERENT | BUS_DMA_NOWAIT),
                        &dma_buf->dma_map);
        if (ret) {
                bus_dma_tag_destroy(dma_buf->dma_tag);
                device_printf(dev, "%s: bus_dmamem_alloc failed\n", __func__);
                goto ql_alloc_dmabuf_exit;
        }

        ret = bus_dmamap_load(dma_buf->dma_tag,
                        dma_buf->dma_map,
                        dma_buf->dma_b,
                        dma_buf->size,
                        qla_dmamap_callback,
                        &b_addr, BUS_DMA_NOWAIT);

        if (ret || !b_addr) {
                bus_dma_tag_destroy(dma_buf->dma_tag);
                bus_dmamem_free(dma_buf->dma_tag, dma_buf->dma_b,
                        dma_buf->dma_map);
                ret = -1;
                goto ql_alloc_dmabuf_exit;
        }

        dma_buf->dma_addr = b_addr;

ql_alloc_dmabuf_exit:
        QL_DPRINT2(ha, (dev, "%s: exit ret 0x%08x tag %p map %p b %p sz 0x%x\n",
                __func__, ret, (void *)dma_buf->dma_tag,
                (void *)dma_buf->dma_map, (void *)dma_buf->dma_b,
		dma_buf->size));

        return ret;
}

void
ql_free_dmabuf(qla_host_t *ha, qla_dma_t *dma_buf)
{
        bus_dmamem_free(dma_buf->dma_tag, dma_buf->dma_b, dma_buf->dma_map);
        bus_dma_tag_destroy(dma_buf->dma_tag);
}

static int
qla_alloc_parent_dma_tag(qla_host_t *ha)
{
	int		ret;
	device_t	dev;

	dev = ha->pci_dev;

        /*
         * Allocate parent DMA Tag
         */
        ret = bus_dma_tag_create(
                        bus_get_dma_tag(dev),   /* parent */
                        1,((bus_size_t)(1ULL << 32)),/* alignment, boundary */
                        BUS_SPACE_MAXADDR,      /* lowaddr */
                        BUS_SPACE_MAXADDR,      /* highaddr */
                        NULL, NULL,             /* filter, filterarg */
                        BUS_SPACE_MAXSIZE_32BIT,/* maxsize */
                        0,                      /* nsegments */
                        BUS_SPACE_MAXSIZE_32BIT,/* maxsegsize */
                        0,                      /* flags */
                        NULL, NULL,             /* lockfunc, lockarg */
                        &ha->parent_tag);

        if (ret) {
                device_printf(dev, "%s: could not create parent dma tag\n",
                        __func__);
		return (-1);
        }

        ha->flags.parent_tag = 1;
	
	return (0);
}

static void
qla_free_parent_dma_tag(qla_host_t *ha)
{
        if (ha->flags.parent_tag) {
                bus_dma_tag_destroy(ha->parent_tag);
                ha->flags.parent_tag = 0;
        }
}

/*
 * Name: qla_init_ifnet
 * Function: Creates the Network Device Interface and Registers it with the O.S
 */

static void
qla_init_ifnet(device_t dev, qla_host_t *ha)
{
	struct ifnet *ifp;

	QL_DPRINT2(ha, (dev, "%s: enter\n", __func__));

	ifp = ha->ifp = if_alloc(IFT_ETHER);

	if (ifp == NULL)
		panic("%s: cannot if_alloc()\n", device_get_nameunit(dev));

	if_initname(ifp, device_get_name(dev), device_get_unit(dev));

	ifp->if_baudrate = IF_Gbps(10);
	ifp->if_capabilities = IFCAP_LINKSTATE;
	ifp->if_mtu = ETHERMTU;

	ifp->if_init = qla_init;
	ifp->if_softc = ha;
	ifp->if_flags = IFF_BROADCAST | IFF_SIMPLEX | IFF_MULTICAST;
	ifp->if_ioctl = qla_ioctl;
	ifp->if_start = qla_start;

	IFQ_SET_MAXLEN(&ifp->if_snd, qla_get_ifq_snd_maxlen(ha));
	ifp->if_snd.ifq_drv_maxlen = qla_get_ifq_snd_maxlen(ha);
	IFQ_SET_READY(&ifp->if_snd);

	ha->max_frame_size = ifp->if_mtu + ETHER_HDR_LEN + ETHER_CRC_LEN;

	ether_ifattach(ifp, qla_get_mac_addr(ha));

	ifp->if_capabilities = IFCAP_HWCSUM |
				IFCAP_TSO4 |
				IFCAP_JUMBO_MTU;

	ifp->if_capabilities |= IFCAP_VLAN_HWTAGGING | IFCAP_VLAN_MTU;
	ifp->if_capabilities |= IFCAP_VLAN_HWTSO;

	ifp->if_capenable = ifp->if_capabilities;

	ifp->if_hdrlen = sizeof(struct ether_vlan_header);

	ifmedia_init(&ha->media, IFM_IMASK, qla_media_change, qla_media_status);

	ifmedia_add(&ha->media, (IFM_ETHER | qla_get_optics(ha) | IFM_FDX), 0,
		NULL);
	ifmedia_add(&ha->media, (IFM_ETHER | IFM_AUTO), 0, NULL);

	ifmedia_set(&ha->media, (IFM_ETHER | IFM_AUTO));

	QL_DPRINT2(ha, (dev, "%s: exit\n", __func__));

	return;
}

static void
qla_init_locked(qla_host_t *ha)
{
	struct ifnet *ifp = ha->ifp;

	qla_stop(ha);

	if (qla_alloc_xmt_bufs(ha) != 0) 
		return;

	qla_confirm_9kb_enable(ha);

	if (qla_alloc_rcv_bufs(ha) != 0)
		return;

	bcopy(IF_LLADDR(ha->ifp), ha->hw.mac_addr, ETHER_ADDR_LEN);

	ifp->if_hwassist = CSUM_TCP | CSUM_UDP | CSUM_TSO;

	ha->flags.stop_rcv = 0;
 	if (ql_init_hw_if(ha) == 0) {
		ifp = ha->ifp;
		ifp->if_drv_flags |= IFF_DRV_RUNNING;
		ifp->if_drv_flags &= ~IFF_DRV_OACTIVE;
		ha->flags.qla_watchdog_pause = 0;
		ha->hw_vlan_tx_frames = 0;
		ha->tx_tso_frames = 0;
		ha->flags.qla_interface_up = 1;
	}

	return;
}

static void
qla_init(void *arg)
{
	qla_host_t *ha;

	ha = (qla_host_t *)arg;

	QL_DPRINT2(ha, (ha->pci_dev, "%s: enter\n", __func__));

	(void)QLA_LOCK(ha, __func__, 0);
	qla_init_locked(ha);
	QLA_UNLOCK(ha, __func__);

	QL_DPRINT2(ha, (ha->pci_dev, "%s: exit\n", __func__));
}

static int
qla_set_multi(qla_host_t *ha, uint32_t add_multi)
{
	uint8_t mta[Q8_MAX_NUM_MULTICAST_ADDRS * Q8_MAC_ADDR_LEN];
	struct ifmultiaddr *ifma;
	int mcnt = 0;
	struct ifnet *ifp = ha->ifp;
	int ret = 0;

	if_maddr_rlock(ifp);

	TAILQ_FOREACH(ifma, &ifp->if_multiaddrs, ifma_link) {

		if (ifma->ifma_addr->sa_family != AF_LINK)
			continue;

		if (mcnt == Q8_MAX_NUM_MULTICAST_ADDRS)
			break;

		bcopy(LLADDR((struct sockaddr_dl *) ifma->ifma_addr),
			&mta[mcnt * Q8_MAC_ADDR_LEN], Q8_MAC_ADDR_LEN);

		mcnt++;
	}

	if_maddr_runlock(ifp);

	if (QLA_LOCK(ha, __func__, 1) == 0) {
		ret = ql_hw_set_multi(ha, mta, mcnt, add_multi);
		QLA_UNLOCK(ha, __func__);
	}

	return (ret);
}

static int
qla_ioctl(struct ifnet *ifp, u_long cmd, caddr_t data)
{
	int ret = 0;
	struct ifreq *ifr = (struct ifreq *)data;
	struct ifaddr *ifa = (struct ifaddr *)data;
	qla_host_t *ha;

	ha = (qla_host_t *)ifp->if_softc;

	switch (cmd) {
	case SIOCSIFADDR:
		QL_DPRINT4(ha, (ha->pci_dev, "%s: SIOCSIFADDR (0x%lx)\n",
			__func__, cmd));

		if (ifa->ifa_addr->sa_family == AF_INET) {
			ifp->if_flags |= IFF_UP;
			if (!(ifp->if_drv_flags & IFF_DRV_RUNNING)) {
				(void)QLA_LOCK(ha, __func__, 0);
				qla_init_locked(ha);
				QLA_UNLOCK(ha, __func__);
			}
			QL_DPRINT4(ha, (ha->pci_dev,
				"%s: SIOCSIFADDR (0x%lx) ipv4 [0x%08x]\n",
				__func__, cmd,
				ntohl(IA_SIN(ifa)->sin_addr.s_addr)));

			arp_ifinit(ifp, ifa);
		} else {
			ether_ioctl(ifp, cmd, data);
		}
		break;

	case SIOCSIFMTU:
		QL_DPRINT4(ha, (ha->pci_dev, "%s: SIOCSIFMTU (0x%lx)\n",
			__func__, cmd));

		if (ifr->ifr_mtu > QLA_MAX_MTU) {
			ret = EINVAL;
		} else {
			(void) QLA_LOCK(ha, __func__, 0);
			ifp->if_mtu = ifr->ifr_mtu;
			ha->max_frame_size =
				ifp->if_mtu + ETHER_HDR_LEN + ETHER_CRC_LEN;
			if ((ifp->if_drv_flags & IFF_DRV_RUNNING)) {
				ret = ql_set_max_mtu(ha, ha->max_frame_size,
					ha->hw.rcv_cntxt_id);
			}

			if (ifp->if_mtu > ETHERMTU)
				ha->std_replenish = QL_JUMBO_REPLENISH_THRES;
			else
				ha->std_replenish = QL_STD_REPLENISH_THRES;
				

			QLA_UNLOCK(ha, __func__);

			if (ret)
				ret = EINVAL;
		}

		break;

	case SIOCSIFFLAGS:
		QL_DPRINT4(ha, (ha->pci_dev, "%s: SIOCSIFFLAGS (0x%lx)\n",
			__func__, cmd));

		(void)QLA_LOCK(ha, __func__, 0);

		if (ifp->if_flags & IFF_UP) {
			if ((ifp->if_drv_flags & IFF_DRV_RUNNING)) {
				if ((ifp->if_flags ^ ha->if_flags) &
					IFF_PROMISC) {
					ret = ql_set_promisc(ha);
				} else if ((ifp->if_flags ^ ha->if_flags) &
					IFF_ALLMULTI) {
					ret = ql_set_allmulti(ha);
				}
			} else {
				qla_init_locked(ha);
				ha->max_frame_size = ifp->if_mtu +
					ETHER_HDR_LEN + ETHER_CRC_LEN;
				ret = ql_set_max_mtu(ha, ha->max_frame_size,
					ha->hw.rcv_cntxt_id);
			}
		} else {
			if (ifp->if_drv_flags & IFF_DRV_RUNNING)
				qla_stop(ha);
			ha->if_flags = ifp->if_flags;
		}

		QLA_UNLOCK(ha, __func__);
		break;

	case SIOCADDMULTI:
		QL_DPRINT4(ha, (ha->pci_dev,
			"%s: %s (0x%lx)\n", __func__, "SIOCADDMULTI", cmd));

		if (ifp->if_drv_flags & IFF_DRV_RUNNING) {
			if (qla_set_multi(ha, 1))
				ret = EINVAL;
		}
		break;

	case SIOCDELMULTI:
		QL_DPRINT4(ha, (ha->pci_dev,
			"%s: %s (0x%lx)\n", __func__, "SIOCDELMULTI", cmd));

		if (ifp->if_drv_flags & IFF_DRV_RUNNING) {
			if (qla_set_multi(ha, 0))
				ret = EINVAL;
		}
		break;

	case SIOCSIFMEDIA:
	case SIOCGIFMEDIA:
		QL_DPRINT4(ha, (ha->pci_dev,
			"%s: SIOCSIFMEDIA/SIOCGIFMEDIA (0x%lx)\n",
			__func__, cmd));
		ret = ifmedia_ioctl(ifp, ifr, &ha->media, cmd);
		break;

	case SIOCSIFCAP:
	{
		int mask = ifr->ifr_reqcap ^ ifp->if_capenable;

		QL_DPRINT4(ha, (ha->pci_dev, "%s: SIOCSIFCAP (0x%lx)\n",
			__func__, cmd));

		if (mask & IFCAP_HWCSUM)
			ifp->if_capenable ^= IFCAP_HWCSUM;
		if (mask & IFCAP_TSO4)
			ifp->if_capenable ^= IFCAP_TSO4;
		if (mask & IFCAP_VLAN_HWTAGGING)
			ifp->if_capenable ^= IFCAP_VLAN_HWTAGGING;
		if (mask & IFCAP_VLAN_HWTSO)
			ifp->if_capenable ^= IFCAP_VLAN_HWTSO;

		if (!(ifp->if_drv_flags & IFF_DRV_RUNNING))
			qla_init(ha);

		VLAN_CAPABILITIES(ifp);
		break;
	}

	default:
		QL_DPRINT4(ha, (ha->pci_dev, "%s: default (0x%lx)\n",
			__func__, cmd));
		ret = ether_ioctl(ifp, cmd, data);
		break;
	}

	return (ret);
}

static int
qla_media_change(struct ifnet *ifp)
{
	qla_host_t *ha;
	struct ifmedia *ifm;
	int ret = 0;

	ha = (qla_host_t *)ifp->if_softc;

	QL_DPRINT2(ha, (ha->pci_dev, "%s: enter\n", __func__));

	ifm = &ha->media;

	if (IFM_TYPE(ifm->ifm_media) != IFM_ETHER)
		ret = EINVAL;

	QL_DPRINT2(ha, (ha->pci_dev, "%s: exit\n", __func__));

	return (ret);
}

static void
qla_media_status(struct ifnet *ifp, struct ifmediareq *ifmr)
{
	qla_host_t *ha;

	ha = (qla_host_t *)ifp->if_softc;

	QL_DPRINT2(ha, (ha->pci_dev, "%s: enter\n", __func__));

	ifmr->ifm_status = IFM_AVALID;
	ifmr->ifm_active = IFM_ETHER;
	
	ql_update_link_state(ha);
	if (ha->hw.link_up) {
		ifmr->ifm_status |= IFM_ACTIVE;
		ifmr->ifm_active |= (IFM_FDX | qla_get_optics(ha));
	}

	QL_DPRINT2(ha, (ha->pci_dev, "%s: exit (%s)\n", __func__,\
		(ha->hw.link_up ? "link_up" : "link_down")));

	return;
}

static void
qla_start(struct ifnet *ifp)
{
	struct mbuf    *m_head;
	qla_host_t *ha = (qla_host_t *)ifp->if_softc;

	QL_DPRINT8(ha, (ha->pci_dev, "%s: enter\n", __func__));

	if (!mtx_trylock(&ha->tx_lock)) {
		QL_DPRINT8(ha, (ha->pci_dev,
			"%s: mtx_trylock(&ha->tx_lock) failed\n", __func__));
		return;
	}

	if ((ifp->if_drv_flags & (IFF_DRV_RUNNING | IFF_DRV_OACTIVE)) != 
		IFF_DRV_RUNNING) {
		QL_DPRINT8(ha,
			(ha->pci_dev, "%s: !IFF_DRV_RUNNING\n", __func__));
		QLA_TX_UNLOCK(ha);
		return;
	}

	if (!ha->hw.link_up || !ha->watchdog_ticks)
		ql_update_link_state(ha);

	if (!ha->hw.link_up) {
		QL_DPRINT8(ha, (ha->pci_dev, "%s: link down\n", __func__));
		QLA_TX_UNLOCK(ha);
		return;
	}

	while (ifp->if_snd.ifq_head != NULL) {
		IF_DEQUEUE(&ifp->if_snd, m_head);

		if (m_head == NULL) {
			QL_DPRINT8(ha, (ha->pci_dev, "%s: m_head == NULL\n",
				__func__));
			break;
		}

		if (qla_send(ha, &m_head)) {
			if (m_head == NULL)
				break;
			QL_DPRINT8(ha, (ha->pci_dev, "%s: PREPEND\n", __func__));
			ifp->if_drv_flags |= IFF_DRV_OACTIVE;
			IF_PREPEND(&ifp->if_snd, m_head);
			break;
		}
		/* Send a copy of the frame to the BPF listener */
		ETHER_BPF_MTAP(ifp, m_head);
	}
	QLA_TX_UNLOCK(ha);
	QL_DPRINT8(ha, (ha->pci_dev, "%s: exit\n", __func__));
	return;
}

static int
qla_send(qla_host_t *ha, struct mbuf **m_headp)
{
	bus_dma_segment_t	segs[QLA_MAX_SEGMENTS];
	bus_dmamap_t		map;
	int			nsegs;
	int			ret = -1;
	uint32_t		tx_idx;
	struct mbuf		*m_head = *m_headp;
	uint32_t		txr_idx = ha->txr_idx;
	uint32_t		iscsi_pdu = 0;

	QL_DPRINT8(ha, (ha->pci_dev, "%s: enter\n", __func__));

	/* check if flowid is set */

	if (M_HASHTYPE_GET(m_head) != M_HASHTYPE_NONE) {
#ifdef QL_ENABLE_ISCSI_TLV
		if (qla_iscsi_pdu(ha, m_head) == 0) {
			iscsi_pdu = 1;
			txr_idx = m_head->m_pkthdr.flowid &
					((ha->hw.num_tx_rings >> 1) - 1);
		} else {
			txr_idx = m_head->m_pkthdr.flowid &
					(ha->hw.num_tx_rings - 1);
		}
#else
		txr_idx = m_head->m_pkthdr.flowid & (ha->hw.num_tx_rings - 1);
#endif /* #ifdef QL_ENABLE_ISCSI_TLV */
	}


	tx_idx = ha->hw.tx_cntxt[txr_idx].txr_next;
	map = ha->tx_ring[txr_idx].tx_buf[tx_idx].map;

	ret = bus_dmamap_load_mbuf_sg(ha->tx_tag, map, m_head, segs, &nsegs,
			BUS_DMA_NOWAIT);

	if (ret == EFBIG) {

		struct mbuf *m;

		QL_DPRINT8(ha, (ha->pci_dev, "%s: EFBIG [%d]\n", __func__,
			m_head->m_pkthdr.len));

		m = m_defrag(m_head, M_NOWAIT);
		if (m == NULL) {
			ha->err_tx_defrag++;
			m_freem(m_head);
			*m_headp = NULL;
			device_printf(ha->pci_dev,
				"%s: m_defrag() = NULL [%d]\n",
				__func__, ret);
			return (ENOBUFS);
		}
		m_head = m;
		*m_headp = m_head;

		if ((ret = bus_dmamap_load_mbuf_sg(ha->tx_tag, map, m_head,
					segs, &nsegs, BUS_DMA_NOWAIT))) {

			ha->err_tx_dmamap_load++;

			device_printf(ha->pci_dev,
				"%s: bus_dmamap_load_mbuf_sg failed0[%d, %d]\n",
				__func__, ret, m_head->m_pkthdr.len);

			if (ret != ENOMEM) {
				m_freem(m_head);
				*m_headp = NULL;
			}
			return (ret);
		}

	} else if (ret) {

		ha->err_tx_dmamap_load++;

		device_printf(ha->pci_dev,
			"%s: bus_dmamap_load_mbuf_sg failed1[%d, %d]\n",
			__func__, ret, m_head->m_pkthdr.len);

		if (ret != ENOMEM) {
			m_freem(m_head);
			*m_headp = NULL;
		}
		return (ret);
	}

	QL_ASSERT(ha, (nsegs != 0), ("qla_send: empty packet"));

	bus_dmamap_sync(ha->tx_tag, map, BUS_DMASYNC_PREWRITE);

        if (!(ret = ql_hw_send(ha, segs, nsegs, tx_idx, m_head, txr_idx,
				iscsi_pdu))) {
		ha->tx_ring[txr_idx].count++;
		ha->tx_ring[txr_idx].tx_buf[tx_idx].m_head = m_head;
	} else {
		if (ret == EINVAL) {
			if (m_head)
				m_freem(m_head);
			*m_headp = NULL;
		}
	}

	QL_DPRINT8(ha, (ha->pci_dev, "%s: exit\n", __func__));
	return (ret);
}

static void
qla_stop(qla_host_t *ha)
{
	struct ifnet *ifp = ha->ifp;
	device_t	dev;

	dev = ha->pci_dev;

	ifp->if_drv_flags &= ~(IFF_DRV_OACTIVE | IFF_DRV_RUNNING);
	QLA_TX_LOCK(ha); QLA_TX_UNLOCK(ha);

	ha->flags.qla_watchdog_pause = 1;

	while (!ha->qla_watchdog_paused)
		qla_mdelay(__func__, 1);

	ha->flags.qla_interface_up = 0;

	ql_hw_stop_rcv(ha);

	ql_del_hw_if(ha);

	qla_free_xmt_bufs(ha);
	qla_free_rcv_bufs(ha);

	return;
}

/*
 * Buffer Management Functions for Transmit and Receive Rings
 */
static int
qla_alloc_xmt_bufs(qla_host_t *ha)
{
	int ret = 0;
	uint32_t i, j;
	qla_tx_buf_t *txb;

	if (bus_dma_tag_create(NULL,    /* parent */
		1, 0,    /* alignment, bounds */
		BUS_SPACE_MAXADDR,       /* lowaddr */
		BUS_SPACE_MAXADDR,       /* highaddr */
		NULL, NULL,      /* filter, filterarg */
		QLA_MAX_TSO_FRAME_SIZE,     /* maxsize */
		QLA_MAX_SEGMENTS,        /* nsegments */
		PAGE_SIZE,        /* maxsegsize */
		BUS_DMA_ALLOCNOW,        /* flags */
		NULL,    /* lockfunc */
		NULL,    /* lockfuncarg */
		&ha->tx_tag)) {
		device_printf(ha->pci_dev, "%s: tx_tag alloc failed\n",
			__func__);
		return (ENOMEM);
	}

	for (i = 0; i < ha->hw.num_tx_rings; i++) {
		bzero((void *)ha->tx_ring[i].tx_buf,
			(sizeof(qla_tx_buf_t) * NUM_TX_DESCRIPTORS));
	}

	for (j = 0; j < ha->hw.num_tx_rings; j++) {
		for (i = 0; i < NUM_TX_DESCRIPTORS; i++) {

			txb = &ha->tx_ring[j].tx_buf[i];

			if ((ret = bus_dmamap_create(ha->tx_tag,
					BUS_DMA_NOWAIT, &txb->map))) {

				ha->err_tx_dmamap_create++;
				device_printf(ha->pci_dev,
					"%s: bus_dmamap_create failed[%d]\n",
					__func__, ret);

				qla_free_xmt_bufs(ha);

				return (ret);
			}
		}
	}

	return 0;
}

/*
 * Release mbuf after it sent on the wire
 */
static void
qla_clear_tx_buf(qla_host_t *ha, qla_tx_buf_t *txb)
{
	QL_DPRINT2(ha, (ha->pci_dev, "%s: enter\n", __func__));

	if (txb->m_head && txb->map) {

		bus_dmamap_unload(ha->tx_tag, txb->map);

		m_freem(txb->m_head);
		txb->m_head = NULL;
	}

	if (txb->map)
		bus_dmamap_destroy(ha->tx_tag, txb->map);

	QL_DPRINT2(ha, (ha->pci_dev, "%s: exit\n", __func__));
}

static void
qla_free_xmt_bufs(qla_host_t *ha)
{
	int		i, j;

	for (j = 0; j < ha->hw.num_tx_rings; j++) {
		for (i = 0; i < NUM_TX_DESCRIPTORS; i++)
			qla_clear_tx_buf(ha, &ha->tx_ring[j].tx_buf[i]);
	}

	if (ha->tx_tag != NULL) {
		bus_dma_tag_destroy(ha->tx_tag);
		ha->tx_tag = NULL;
	}

	for (i = 0; i < ha->hw.num_tx_rings; i++) {
		bzero((void *)ha->tx_ring[i].tx_buf,
			(sizeof(qla_tx_buf_t) * NUM_TX_DESCRIPTORS));
	}
	return;
}


static int
qla_alloc_rcv_std(qla_host_t *ha)
{
	int		i, j, k, r, ret = 0;
	qla_rx_buf_t	*rxb;
	qla_rx_ring_t	*rx_ring;

	for (r = 0; r < ha->hw.num_rds_rings; r++) {

		rx_ring = &ha->rx_ring[r];

		for (i = 0; i < NUM_RX_DESCRIPTORS; i++) {

			rxb = &rx_ring->rx_buf[i];

			ret = bus_dmamap_create(ha->rx_tag, BUS_DMA_NOWAIT,
					&rxb->map);

			if (ret) {
				device_printf(ha->pci_dev,
					"%s: dmamap[%d, %d] failed\n",
					__func__, r, i);

				for (k = 0; k < r; k++) {
					for (j = 0; j < NUM_RX_DESCRIPTORS;
						j++) {
						rxb = &ha->rx_ring[k].rx_buf[j];
						bus_dmamap_destroy(ha->rx_tag,
							rxb->map);
					}
				}

				for (j = 0; j < i; j++) {
					bus_dmamap_destroy(ha->rx_tag,
						rx_ring->rx_buf[j].map);
				}
				goto qla_alloc_rcv_std_err;
			}
		}
	}

	qla_init_hw_rcv_descriptors(ha);

	
	for (r = 0; r < ha->hw.num_rds_rings; r++) {

		rx_ring = &ha->rx_ring[r];

		for (i = 0; i < NUM_RX_DESCRIPTORS; i++) {
			rxb = &rx_ring->rx_buf[i];
			rxb->handle = i;
			if (!(ret = ql_get_mbuf(ha, rxb, NULL))) {
				/*
			 	 * set the physical address in the
				 * corresponding descriptor entry in the
				 * receive ring/queue for the hba 
				 */
				qla_set_hw_rcv_desc(ha, r, i, rxb->handle,
					rxb->paddr,
					(rxb->m_head)->m_pkthdr.len);
			} else {
				device_printf(ha->pci_dev,
					"%s: ql_get_mbuf [%d, %d] failed\n",
					__func__, r, i);
				bus_dmamap_destroy(ha->rx_tag, rxb->map);
				goto qla_alloc_rcv_std_err;
			}
		}
	}
	return 0;

qla_alloc_rcv_std_err:
	return (-1);
}

static void
qla_free_rcv_std(qla_host_t *ha)
{
	int		i, r;
	qla_rx_buf_t	*rxb;

	for (r = 0; r < ha->hw.num_rds_rings; r++) {
		for (i = 0; i < NUM_RX_DESCRIPTORS; i++) {
			rxb = &ha->rx_ring[r].rx_buf[i];
			if (rxb->m_head != NULL) {
				bus_dmamap_unload(ha->rx_tag, rxb->map);
				bus_dmamap_destroy(ha->rx_tag, rxb->map);
				m_freem(rxb->m_head);
				rxb->m_head = NULL;
			}
		}
	}
	return;
}

static int
qla_alloc_rcv_bufs(qla_host_t *ha)
{
	int		i, ret = 0;

	if (bus_dma_tag_create(NULL,    /* parent */
			1, 0,    /* alignment, bounds */
			BUS_SPACE_MAXADDR,       /* lowaddr */
			BUS_SPACE_MAXADDR,       /* highaddr */
			NULL, NULL,      /* filter, filterarg */
			MJUM9BYTES,     /* maxsize */
			1,        /* nsegments */
			MJUM9BYTES,        /* maxsegsize */
			BUS_DMA_ALLOCNOW,        /* flags */
			NULL,    /* lockfunc */
			NULL,    /* lockfuncarg */
			&ha->rx_tag)) {

		device_printf(ha->pci_dev, "%s: rx_tag alloc failed\n",
			__func__);

		return (ENOMEM);
	}

	bzero((void *)ha->rx_ring, (sizeof(qla_rx_ring_t) * MAX_RDS_RINGS));

	for (i = 0; i < ha->hw.num_sds_rings; i++) {
		ha->hw.sds[i].sdsr_next = 0;
		ha->hw.sds[i].rxb_free = NULL;
		ha->hw.sds[i].rx_free = 0;
	}

	ret = qla_alloc_rcv_std(ha);

	return (ret);
}

static void
qla_free_rcv_bufs(qla_host_t *ha)
{
	int		i;

	qla_free_rcv_std(ha);

	if (ha->rx_tag != NULL) {
		bus_dma_tag_destroy(ha->rx_tag);
		ha->rx_tag = NULL;
	}

	bzero((void *)ha->rx_ring, (sizeof(qla_rx_ring_t) * MAX_RDS_RINGS));

	for (i = 0; i < ha->hw.num_sds_rings; i++) {
		ha->hw.sds[i].sdsr_next = 0;
		ha->hw.sds[i].rxb_free = NULL;
		ha->hw.sds[i].rx_free = 0;
	}

	return;
}

int
ql_get_mbuf(qla_host_t *ha, qla_rx_buf_t *rxb, struct mbuf *nmp)
{
	register struct mbuf *mp = nmp;
	struct ifnet   		*ifp;
	int            		ret = 0;
	uint32_t		offset;
	bus_dma_segment_t	segs[1];
	int			nsegs, mbuf_size;

	QL_DPRINT2(ha, (ha->pci_dev, "%s: enter\n", __func__));

	ifp = ha->ifp;

        if (ha->hw.enable_9kb)
                mbuf_size = MJUM9BYTES;
        else
                mbuf_size = MCLBYTES;

	if (mp == NULL) {

		if (QL_ERR_INJECT(ha, INJCT_M_GETCL_M_GETJCL_FAILURE))
			return(-1);

                if (ha->hw.enable_9kb)
                        mp = m_getjcl(M_NOWAIT, MT_DATA, M_PKTHDR, mbuf_size);
                else
                        mp = m_getcl(M_NOWAIT, MT_DATA, M_PKTHDR);

		if (mp == NULL) {
			ha->err_m_getcl++;
			ret = ENOBUFS;
			device_printf(ha->pci_dev,
					"%s: m_getcl failed\n", __func__);
			goto exit_ql_get_mbuf;
		}
		mp->m_len = mp->m_pkthdr.len = mbuf_size;
	} else {
		mp->m_len = mp->m_pkthdr.len = mbuf_size;
		mp->m_data = mp->m_ext.ext_buf;
		mp->m_next = NULL;
	}

	offset = (uint32_t)((unsigned long long)mp->m_data & 0x7ULL);
	if (offset) {
		offset = 8 - offset;
		m_adj(mp, offset);
	}

	/*
	 * Using memory from the mbuf cluster pool, invoke the bus_dma
	 * machinery to arrange the memory mapping.
	 */
	ret = bus_dmamap_load_mbuf_sg(ha->rx_tag, rxb->map,
			mp, segs, &nsegs, BUS_DMA_NOWAIT);
	rxb->paddr = segs[0].ds_addr;

	if (ret || !rxb->paddr || (nsegs != 1)) {
		m_free(mp);
		rxb->m_head = NULL;
		device_printf(ha->pci_dev,
			"%s: bus_dmamap_load failed[%d, 0x%016llx, %d]\n",
			__func__, ret, (long long unsigned int)rxb->paddr,
			nsegs);
                ret = -1;
		goto exit_ql_get_mbuf;
	}
	rxb->m_head = mp;
	bus_dmamap_sync(ha->rx_tag, rxb->map, BUS_DMASYNC_PREREAD);

exit_ql_get_mbuf:
	QL_DPRINT2(ha, (ha->pci_dev, "%s: exit ret = 0x%08x\n", __func__, ret));
	return (ret);
}

static void
qla_tx_done(void *context, int pending)
{
	qla_host_t *ha = context;
	struct ifnet   *ifp;

	ifp = ha->ifp;

	if (!ifp) 
		return;

	if (!(ifp->if_drv_flags & IFF_DRV_RUNNING)) {
		QL_DPRINT8(ha, (ha->pci_dev, "%s: !IFF_DRV_RUNNING\n", __func__));
		return;
	}
	ql_hw_tx_done(ha);

	qla_start(ha->ifp);
}

static void
qla_get_peer(qla_host_t *ha)
{
	device_t *peers;
	int count, i, slot;
	int my_slot = pci_get_slot(ha->pci_dev);

	if (device_get_children(device_get_parent(ha->pci_dev), &peers, &count))
		return;

	for (i = 0; i < count; i++) {
		slot = pci_get_slot(peers[i]);

		if ((slot >= 0) && (slot == my_slot) &&
			(pci_get_device(peers[i]) ==
				pci_get_device(ha->pci_dev))) {
			if (ha->pci_dev != peers[i]) 
				ha->peer_dev = peers[i];
		}
	}
}

static void
qla_send_msg_to_peer(qla_host_t *ha, uint32_t msg_to_peer)
{
	qla_host_t *ha_peer;
	
	if (ha->peer_dev) {
        	if ((ha_peer = device_get_softc(ha->peer_dev)) != NULL) {

			ha_peer->msg_from_peer = msg_to_peer;
		}
	}
}

static void
qla_error_recovery(void *context, int pending)
{
	qla_host_t *ha = context;
	uint32_t msecs_100 = 100;
	struct ifnet *ifp = ha->ifp;

        (void)QLA_LOCK(ha, __func__, 0);

	if (ha->flags.qla_interface_up) {

	ha->hw.imd_compl = 1;
	qla_mdelay(__func__, 300);

        ql_hw_stop_rcv(ha);

        ifp->if_drv_flags &= ~(IFF_DRV_OACTIVE | IFF_DRV_RUNNING);
		QLA_TX_LOCK(ha); QLA_TX_UNLOCK(ha);
	}

        QLA_UNLOCK(ha, __func__);

	if ((ha->pci_func & 0x1) == 0) {

		if (!ha->msg_from_peer) {
			qla_send_msg_to_peer(ha, QL_PEER_MSG_RESET);

			while ((ha->msg_from_peer != QL_PEER_MSG_ACK) &&
				msecs_100--)
				qla_mdelay(__func__, 100);
		}

		ha->msg_from_peer = 0;

        	(void)QLA_LOCK(ha, __func__, 0);
		ql_minidump(ha);
        	QLA_UNLOCK(ha, __func__);

		(void) ql_init_hw(ha);

        	(void)QLA_LOCK(ha, __func__, 0);
		if (ha->flags.qla_interface_up) {
        	qla_free_xmt_bufs(ha);
	        qla_free_rcv_bufs(ha);
		}
        	QLA_UNLOCK(ha, __func__);

		qla_send_msg_to_peer(ha, QL_PEER_MSG_ACK);

	} else {
		if (ha->msg_from_peer == QL_PEER_MSG_RESET) {

			ha->msg_from_peer = 0;

			qla_send_msg_to_peer(ha, QL_PEER_MSG_ACK);
		} else {
			qla_send_msg_to_peer(ha, QL_PEER_MSG_RESET);
		}

		while ((ha->msg_from_peer != QL_PEER_MSG_ACK)  && msecs_100--)
			qla_mdelay(__func__, 100);
		ha->msg_from_peer = 0;

		(void) ql_init_hw(ha);

        	(void)QLA_LOCK(ha, __func__, 0);
		if (ha->flags.qla_interface_up) {
        	qla_free_xmt_bufs(ha);
	        qla_free_rcv_bufs(ha);
	}
        	QLA_UNLOCK(ha, __func__);
	}

        (void)QLA_LOCK(ha, __func__, 0);

	if (ha->flags.qla_interface_up) {
	if (qla_alloc_xmt_bufs(ha) != 0) {
        	QLA_UNLOCK(ha, __func__);
                return;
	}
	qla_confirm_9kb_enable(ha);

        if (qla_alloc_rcv_bufs(ha) != 0) {
        	QLA_UNLOCK(ha, __func__);
                return;
	}

        ha->flags.stop_rcv = 0;
        if (ql_init_hw_if(ha) == 0) {
                ifp = ha->ifp;
                ifp->if_drv_flags |= IFF_DRV_RUNNING;
                ifp->if_drv_flags &= ~IFF_DRV_OACTIVE;
                ha->flags.qla_watchdog_pause = 0;
        }
	} else
		ha->flags.qla_watchdog_pause = 0;

        QLA_UNLOCK(ha, __func__);
}

static void
qla_async_event(void *context, int pending)
{
        qla_host_t *ha = context;

        (void)QLA_LOCK(ha, __func__, 0);
        qla_hw_async_event(ha);
        QLA_UNLOCK(ha, __func__);
}

