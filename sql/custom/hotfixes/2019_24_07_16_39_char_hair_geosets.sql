DROP TABLE IF EXISTS `char_hair_geosets`;
CREATE TABLE IF NOT EXISTS `char_hair_geosets` (
  `ID` int(10) unsigned NOT NULL DEFAULT '0',
  `RaceID` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `SexID` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `VariationID` tinyint(3) NOT NULL DEFAULT '0',
  `GeosetID` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `Showscalp` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `VariationType` tinyint(3) NOT NULL DEFAULT '0',
  `GeosetType` tinyint(3) NOT NULL DEFAULT '0',
  `ColorIndex` tinyint(3) NOT NULL DEFAULT '0',
  `CustomGeoFileDataID` int(10) NOT NULL DEFAULT '0',
  `HdCustomGeoFileDataID` int(10) NOT NULL DEFAULT '0',
  `unk83` tinyint(4) DEFAULT NULL,
  `VerifiedBuild` int(11) NOT NULL DEFAULT '35284',
  PRIMARY KEY (`ID`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;