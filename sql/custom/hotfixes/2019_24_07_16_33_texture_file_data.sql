DROP TABLE IF EXISTS `texture_file_data`;
CREATE TABLE IF NOT EXISTS `texture_file_data` (
  `ID` int(10) unsigned NOT NULL,
  `UsageType` tinyint(2) unsigned NOT NULL DEFAULT '0',
  `TextureID` int(10) NOT NULL DEFAULT '0',
  `VerifiedBuild` int(11) NOT NULL DEFAULT '35284',
  PRIMARY KEY (`ID`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;