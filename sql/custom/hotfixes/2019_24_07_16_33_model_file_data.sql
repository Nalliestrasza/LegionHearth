DROP TABLE IF EXISTS `model_file_data`;
CREATE TABLE IF NOT EXISTS `model_file_data` (
  `ID` int(10) unsigned NOT NULL,
  `unk0` tinyint(2) unsigned NOT NULL DEFAULT '0',
  `unk1` tinyint(2) unsigned NOT NULL DEFAULT '0',
  `ModelID` int(10) unsigned NOT NULL DEFAULT '0',
  `VerifiedBuild` int(11) NOT NULL DEFAULT '35284',
  PRIMARY KEY (`ID`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;