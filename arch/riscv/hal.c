void hal_reboot() {
    sbi_system_reset(SBI_SRST_TYPE_COLD_REBOOT, SBI_SRST_REASON_NONE);
}
