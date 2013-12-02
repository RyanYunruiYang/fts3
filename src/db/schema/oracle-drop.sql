drop trigger se_id_info_auto_inc;
drop sequence file_file_id_seq;
drop trigger file_file_id_auto_inc;
drop sequence se_id_info_seq;
drop trigger t_optimize_auto_number_inc;
drop sequence t_optimize_auto_number_seq;


DROP TABLE t_credential;
DROP TABLE t_credential_cache;
DROP TABLE t_credential_vers;
drop table T_BAD_SES cascade constraints;
drop table T_BAD_DNS cascade constraints;
drop table T_FILE cascade constraints;
drop table T_JOB cascade constraints;
drop table T_SCHEMA_VERS cascade constraints;
drop table T_SE cascade constraints;
drop table T_SE_ACL cascade constraints;
drop table T_SE_PAIR_ACL cascade constraints;
drop table T_STAGE_REQ cascade constraints;
drop table T_VO_ACL cascade constraints;
drop table T_DEBUG cascade constraints;
drop table T_CONFIG_AUDIT cascade constraints;
drop table t_optimizer_evolution;
drop table t_optimize cascade constraints;
drop table t_optimize_mode;
drop table t_file_backup;
drop table t_job_backup;
drop table T_GROUP_MEMBERS cascade constraints;
drop table T_SHARE_CONFIG cascade constraints;
drop table T_LINK_CONFIG cascade constraints;
drop table T_SERVER_CONFIG cascade constraints;
drop table T_FILE_SHARE_CONFIG  cascade constraints;
drop table t_file_retry_errors  cascade constraints;

drop table t_profiling_info;
drop table t_profiling_snapshot;
drop table t_server_sanity;

exit;
