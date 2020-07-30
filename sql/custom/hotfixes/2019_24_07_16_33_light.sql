CREATE TABLE IF NOT EXISTS `light` (
  `ID` int(10) unsigned NOT NULL DEFAULT '0',
  `GameCoordsX` float NOT NULL DEFAULT '0',
  `GameCoordsY` float NOT NULL DEFAULT '0',
  `GameCoordsZ` float NOT NULL DEFAULT '0',
  `GameFalloffStart` float NOT NULL DEFAULT '0',
  `GameFalloffEnd` float NOT NULL DEFAULT '0',
  `ContinentID` smallint(6) NOT NULL DEFAULT '0',
  `LightParamsID1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `LightParamsID2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `LightParamsID3` smallint(5) unsigned NOT NULL DEFAULT '0',
  `LightParamsID4` smallint(5) unsigned NOT NULL DEFAULT '0',
  `LightParamsID5` smallint(5) unsigned NOT NULL DEFAULT '0',
  `LightParamsID6` smallint(5) unsigned NOT NULL DEFAULT '0',
  `LightParamsID7` smallint(5) unsigned NOT NULL DEFAULT '0',
  `LightParamsID8` smallint(5) unsigned NOT NULL DEFAULT '0',
  `VerifiedBuild` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;