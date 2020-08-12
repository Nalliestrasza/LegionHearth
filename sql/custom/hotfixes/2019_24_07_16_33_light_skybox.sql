CREATE TABLE IF NOT EXISTS `light_skybox` (
  `Id` int(10) unsigned NOT NULL DEFAULT '0',
  `Name` mediumtext CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `Flags` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `SkyboxFileDataID` int(11) NOT NULL DEFAULT '0',
  `CelestialSkyboxFileDataID` int(11) NOT NULL DEFAULT '0',
  `VerifiedBuild` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`Id`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;