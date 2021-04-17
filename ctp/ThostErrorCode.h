#pragma once 
 enum CtpErrorCode{ 
	CTP_NONE = 0,
	CTP_INVALID_DATA_SYNC_STATUS = 1,
	CTP_INCONSISTENT_INFORMATION = 2,
	CTP_INVALID_LOGIN = 3,
	CTP_USER_NOT_ACTIVE = 4,
	CTP_DUPLICATE_LOGIN = 5,
	CTP_NOT_LOGIN_YET = 6,
	CTP_NOT_INITED = 7,
	CTP_FRONT_NOT_ACTIVE = 8,
	CTP_NO_PRIVILEGE = 9,
	CTP_CHANGE_OTHER_PASSWORD = 10,
	CTP_USER_NOT_FOUND = 11,
	CTP_BROKER_NOT_FOUND = 12,
	CTP_INVESTOR_NOT_FOUND = 13,
	CTP_OLD_PASSWORD_MISMATCH = 14,
	CTP_BAD_FIELD = 15,
	CTP_INSTRUMENT_NOT_FOUND = 16,
	CTP_INSTRUMENT_NOT_TRADING = 17,
	CTP_NOT_EXCHANGE_PARTICIPANT = 18,
	CTP_INVESTOR_NOT_ACTIVE = 19,
	CTP_NOT_EXCHANGE_CLIENT = 20,
	CTP_NO_VALID_TRADER_AVAILABLE = 21,
	CTP_DUPLICATE_ORDER_REF = 22,
	CTP_BAD_ORDER_ACTION_FIELD = 23,
	CTP_DUPLICATE_ORDER_ACTION_REF = 24,
	CTP_ORDER_NOT_FOUND = 25,
	CTP_INSUITABLE_ORDER_STATUS = 26,
	CTP_UNSUPPORTED_FUNCTION = 27,
	CTP_NO_TRADING_RIGHT = 28,
	CTP_CLOSE_ONLY = 29,
	CTP_OVER_CLOSE_POSITION = 30,
	CTP_INSUFFICIENT_MONEY = 31,
	CTP_DUPLICATE_PK = 32,
	CTP_CANNOT_FIND_PK = 33,
	CTP_CAN_NOT_INACTIVE_BROKER = 34,
	CTP_BROKER_SYNCHRONIZING = 35,
	CTP_BROKER_SYNCHRONIZED = 36,
	CTP_SHORT_SELL = 37,
	CTP_INVALID_SETTLEMENT_REF = 38,
	CTP_CFFEX_NETWORK_ERROR = 39,
	CTP_CFFEX_OVER_REQUEST = 40,
	CTP_CFFEX_OVER_REQUEST_PER_SECOND = 41,
	CTP_SETTLEMENT_INFO_NOT_CONFIRMED = 42,
	CTP_DEPOSIT_NOT_FOUND = 43,
	CTP_EXCHANG_TRADING = 44,
	CTP_PARKEDORDER_NOT_FOUND = 45,
	CTP_PARKEDORDER_HASSENDED = 46,
	CTP_PARKEDORDER_HASDELETE = 47,
	CTP_INVALID_INVESTORIDORPASSWORD = 48,
	CTP_INVALID_LOGIN_IPADDRESS = 49,
	CTP_OVER_CLOSETODAY_POSITION = 50,
	CTP_OVER_CLOSEYESTERDAY_POSITION = 51,
	CTP_BROKER_NOT_ENOUGH_CONDORDER = 52,
	CTP_INVESTOR_NOT_ENOUGH_CONDORDER = 53,
	CTP_BROKER_NOT_SUPPORT_CONDORDER = 54,
	CTP_RESEND_ORDER_BROKERINVESTOR_NOTMATCH = 55,
	CTP_SYC_OTP_FAILED = 56,
	CTP_OTP_MISMATCH = 57,
	CTP_OTPPARAM_NOT_FOUND = 58,
	CTP_UNSUPPORTED_OTPTYPE = 59,
	CTP_SINGLEUSERSESSION_EXCEED_LIMIT = 60,
	CTP_EXCHANGE_UNSUPPORTED_ARBITRAGE = 61,
	CTP_NO_CONDITIONAL_ORDER_RIGHT = 62,
	CTP_AUTH_FAILED = 63,
	CTP_NOT_AUTHENT = 64,
	CTP_SWAPORDER_UNSUPPORTED = 65,
	CTP_OPTIONS_ONLY_SUPPORT_SPEC = 66,
	CTP_DUPLICATE_EXECORDER_REF = 67,
	CTP_RESEND_EXECORDER_BROKERINVESTOR_NOTMATCH = 68,
	CTP_EXECORDER_NOTOPTIONS = 69,
	CTP_OPTIONS_NOT_SUPPORT_EXEC = 70,
	CTP_BAD_EXECORDER_ACTION_FIELD = 71,
	CTP_DUPLICATE_EXECORDER_ACTION_REF = 72,
	CTP_EXECORDER_NOT_FOUND = 73,
	CTP_OVER_EXECUTE_POSITION = 74,
	CTP_LOGIN_FORBIDDEN = 75,
	CTP_INVALID_TRANSFER_AGENT = 76,
	CTP_NO_FOUND_FUNCTION = 77,
	CTP_SEND_EXCHANGEORDER_FAILED = 78,
	CTP_SEND_EXCHANGEORDERACTION_FAILED = 79,
	CTP_PRICETYPE_NOTSUPPORT_BYEXCHANGE = 80,
	CTP_BAD_EXECUTE_TYPE = 81,
	CTP_BAD_OPTION_INSTR = 82,
	CTP_INSTR_NOTSUPPORT_FORQUOTE = 83,
	CTP_RESEND_QUOTE_BROKERINVESTOR_NOTMATCH = 84,
	CTP_INSTR_NOTSUPPORT_QUOTE = 85,
	CTP_QUOTE_NOT_FOUND = 86,
	CTP_OPTIONS_NOT_SUPPORT_ABANDON = 87,
	CTP_COMBOPTIONS_SUPPORT_IOC_ONLY = 88,
	CTP_OPEN_FILE_FAILED = 89,
	CTP_NEED_RETRY = 90,
	CTP_EXCHANGE_RTNERROR = 91,
	CTP_QUOTE_DERIVEDORDER_ACTIONERROR = 92,
	CTP_INSTRUMENTMAP_NOT_FOUND = 93,
	CTP_SPEC_COVERED_ONLY = 94,
	CTP_SPEC_OPEN_ONLY = 95,
	CTP_DUP_INSTRUMENT = 96,
	CTP_DUP_ORDER = 97,
	CTP_NOT_ENOUGH_LOCKPOSITION = 98,
	CTP_NOT_ENOUGH_STOCKPOSITION = 99,
	CTP_NOT_SUPPORT_LOCK = 100,
	CTP_NO_TRADING_RIGHT_IN_SEPC_DR = 101,
	CTP_NO_DR_NO = 102,
	CTP_COVER_ONLY_SELL = 103,
	CTP_VOLUME_NOTSUPPORT = 104,
	CTP_PRICE_NOTSUPPORT = 105,
	CTP_POSI_LIMIT = 106,
	CTP_BROKERPOSI_LIMIT = 107,
	CTP_BAD_STOCKDISPOSAL_ACTION_FIELD = 108,
	CTP_DUPLICATE_STOCKDISPOSAL_ACTION_REF = 109,
	CTP_STOCKDISPOSAL_NOT_FOUND = 110,
	CTP_NO_INSTRUCTION_RIGHT = 111,
	CTP_NO_RIGHT_LEVEL = 112,
	CTP_OUT_OF_TIMEINTERVAL = 113,
	CTP_OUT_OF_PRICEINTERVAL = 114,
	CTP_SEND_INSTITUTION_CODE_ERROR = 1000,
	CTP_NO_GET_PLATFORM_SN = 1001,
	CTP_ILLEGAL_TRANSFER_BANK = 1002,
	CTP_ALREADY_OPEN_ACCOUNT = 1003,
	CTP_NOT_OPEN_ACCOUNT = 1004,
	CTP_PROCESSING = 1005,
	CTP_OVERTIME = 1006,
	CTP_RECORD_NOT_FOUND = 1007,
	CTP_NO_FOUND_REVERSAL_ORIGINAL_TRANSACTION = 1008,
	CTP_CONNECT_HOST_FAILED = 1009,
	CTP_SEND_FAILED = 1010,
	CTP_LATE_RESPONSE = 1011,
	CTP_REVERSAL_BANKID_NOT_MATCH = 1012,
	CTP_REVERSAL_BANKACCOUNT_NOT_MATCH = 1013,
	CTP_REVERSAL_BROKERID_NOT_MATCH = 1014,
	CTP_REVERSAL_ACCOUNTID_NOT_MATCH = 1015,
	CTP_REVERSAL_AMOUNT_NOT_MATCH = 1016,
	CTP_DB_OPERATION_FAILED = 1017,
	CTP_SEND_ASP_FAILURE = 1018,
	CTP_NOT_SIGNIN = 1019,
	CTP_ALREADY_SIGNIN = 1020,
	CTP_AMOUNT_OR_TIMES_OVER = 1021,
	CTP_NOT_IN_TRANSFER_TIME = 1022,
	CTP_BANK_SERVER_ERROR = 1023,
	CTP_BANK_SERIAL_IS_REPEALED = 1024,
	CTP_BANK_SERIAL_NOT_EXIST = 1025,
	CTP_NOT_ORGAN_MAP = 1026,
	CTP_EXIST_TRANSFER = 1027,
	CTP_BANK_FORBID_REVERSAL = 1028,
	CTP_DUP_BANK_SERIAL = 1029,
	CTP_FBT_SYSTEM_BUSY = 1030,
	CTP_MACKEY_SYNCING = 1031,
	CTP_ACCOUNTID_ALREADY_REGISTER = 1032,
	CTP_BANKACCOUNT_ALREADY_REGISTER = 1033,
	CTP_DUP_BANK_SERIAL_REDO_OK = 1034,
	CTP_CURRENCYID_NOT_SUPPORTED = 1035,
	CTP_INVALID_MAC = 1036,
	CTP_NOT_SUPPORT_SECAGENT_BY_BANK = 1037,
	CTP_PINKEY_SYNCING = 1038,
	CTP_SECAGENT_QUERY_BY_CCB = 1039,
	CTP_ALREADY_SIGNOUT = 1040,
	CTP_NO_WORKKEY = 1041,
	CTP_NO_VALID_BANKOFFER_AVAILABLE = 2000,
	CTP_PASSWORD_MISMATCH = 2001,
	CTP_DUPLATION_BANK_SERIAL = 2004,
	CTP_DUPLATION_OFFER_SERIAL = 2005,
	CTP_SERIAL_NOT_EXSIT = 2006,
	CTP_SERIAL_IS_REPEALED = 2007,
	CTP_SERIAL_MISMATCH = 2008,
	CTP_IdentifiedCardNo_MISMATCH = 2009,
	CTP_ACCOUNT_NOT_FUND = 2011,
	CTP_ACCOUNT_NOT_ACTIVE = 2012,
	CTP_NOT_ALLOW_REPEAL_BYMANUAL = 2013,
	CTP_AMOUNT_OUTOFTHEWAY = 2014,
	CTP_EXCHANGERATE_NOT_FOUND = 2015,
	CTP_WAITING_OFFER_RSP = 999999,
	CTP_FBE_NO_GET_PLATFORM_SN = 3001,
	CTP_FBE_ILLEGAL_TRANSFER_BANK = 3002,
	CTP_FBE_PROCESSING = 3005,
	CTP_FBE_OVERTIME = 3006,
	CTP_FBE_RECORD_NOT_FOUND = 3007,
	CTP_FBE_CONNECT_HOST_FAILED = 3009,
	CTP_FBE_SEND_FAILED = 3010,
	CTP_FBE_LATE_RESPONSE = 3011,
	CTP_FBE_DB_OPERATION_FAILED = 3017,
	CTP_FBE_NOT_SIGNIN = 3019,
	CTP_FBE_ALREADY_SIGNIN = 3020,
	CTP_FBE_AMOUNT_OR_TIMES_OVER = 3021,
	CTP_FBE_NOT_IN_TRANSFER_TIME = 3022,
	CTP_FBE_BANK_SERVER_ERROR = 3023,
	CTP_FBE_NOT_ORGAN_MAP = 3026,
	CTP_FBE_SYSTEM_BUSY = 3030,
	CTP_FBE_CURRENCYID_NOT_SUPPORTED = 3035,
	CTP_FBE_WRONG_BANK_ACCOUNT = 3036,
	CTP_FBE_BANK_ACCOUNT_NO_FUNDS = 3037,
	CTP_FBE_DUP_CERT_NO = 3038,

 };
const char* ctp_strerror(int code);
const char* ctp_strerror_cn(int code);