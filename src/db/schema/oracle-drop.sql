DROP TABLE t_credential;
DROP TABLE t_credential_cache;
DROP TABLE t_credential_vers;
DROP SEQUENCE se_bad_id_seq;
DROP SEQUENCE se_id_info_seq;
DROP SEQUENCE file_file_id_seq;
DROP SEQUENCE se_row_id_seq;
drop table T_BAD_SES cascade constraints;
drop table T_FILE cascade constraints;
drop table T_JOB cascade constraints;
drop table T_SCHEMA_VERS cascade constraints;
drop table T_LOG cascade constraints;
drop table T_SE cascade constraints;
drop table T_SE_GROUP cascade constraints;
drop table T_SE_ACL cascade constraints;
drop table T_SE_PROTOCOL cascade constraints;
drop table T_SE_PAIR_ACL cascade constraints;
drop table T_SE_VO_SHARE cascade constraints;
drop table T_SITE_GROUP cascade constraints;
drop table T_STAGE_REQ cascade constraints;
drop table T_VO_ACL cascade constraints;
drop table T_DEBUG cascade constraints;
drop table T_CONFIG_AUDIT cascade constraints;
drop table t_optimize cascade constraints;
drop table T_FILE_backup cascade constraints;
drop table T_JOB_backup cascade constraints;
exit;
