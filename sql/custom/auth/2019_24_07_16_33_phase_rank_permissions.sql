CREATE TABLE IF NOT EXISTS `phase_rank_permissions` (
  `phaseId` int(10) unsigned DEFAULT NULL,
  `rankName` text,
  `permissions` varbinary(16) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;