CREATE TABLE IF NOT EXISTS `phase_account_permissions` (
  `accountId` int(11) unsigned NOT NULL DEFAULT '0',
  `phaseId` int(11) unsigned DEFAULT NULL,
  `permissions` varbinary(16) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;