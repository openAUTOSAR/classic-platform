// Compliant, reported
/*lint -esym(9003, Crc_16_Tab) MISRA:OTHER:Readability:[MISRA 2012 Rule 8.9, advisory] */

crc = (TRUE == Crc_IsFirstCall) ? Crc_16_CCITTStartValue : (Crc_StartValue16 ^ Crc_16_CCITTXor); /*lint !e835 LINT:OTHER:Readability */

/*lint -e{533, 578, 715, 745, 832, 936, 939} FALSE_POSITIVE */
TASK(SchM_Partition_A0) {
    
            /*lint -save -e747 STANDARDIZED_API */
        SYS_CALL_WaitEvent(EVENT_MASK_Alarm_BswServices_Partition_A0 | EVENT_MASK_DetReportError_QM | EVENT_MASK_SynchPartition_A0);
        SYS_CALL_GetEvent(TASK_ID_SchM_Partition_A0, &Event);
        /*lint -restore */
        
                    SYS_CALL_ClearEvent(EVENT_MASK_DetReportError_QM); /*lint !e747 STANDARDIZED_API */
                    
                        } while (SCHM_TASK_EXTENDED_CONDITION != 0);  /*lint !e506 OTHER Required to loop task in order to synchronize partitions */