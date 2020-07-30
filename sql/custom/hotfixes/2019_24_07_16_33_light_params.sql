CREATE TABLE IF NOT EXISTS `light_params` (
  `OverrideCelestialSphere1` float DEFAULT NULL,
  `OverrideCelestialSphere2` float DEFAULT NULL,
  `OverrideCelestialSphere3` float DEFAULT NULL,
  `ID` int(10) unsigned NOT NULL DEFAULT '0',
  `HighlightSky` tinyint(3) unsigned DEFAULT '0',
  `LightSkyboxID` smallint(5) unsigned DEFAULT '0',
  `CloudTypeID` tinyint(3) unsigned DEFAULT '0',
  `Glow` float unsigned DEFAULT NULL,
  `WaterShallowAlpha` float unsigned DEFAULT NULL,
  `WaterDeepAlpha` float unsigned DEFAULT NULL,
  `OceanShallowAlpha` float unsigned DEFAULT NULL,
  `OceanDeepAlpha` float unsigned DEFAULT NULL,
  `Flags` int(11) DEFAULT '0',
  `SsaoSettingsID` int(11) DEFAULT '0',
  `VerifiedBuild` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;