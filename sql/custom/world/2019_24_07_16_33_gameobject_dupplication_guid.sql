CREATE TABLE IF NOT EXISTS `gameobject_dupplication_guid` (
  `guid` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `entry` int(11) unsigned NOT NULL,
  `refObjGuid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `nom` varchar(50) DEFAULT NULL,
  `spawner` varchar(50) DEFAULT NULL,
  `posX` float NOT NULL,
  `posY` float NOT NULL,
  `posZ` float NOT NULL,
  `mapID` smallint(5) unsigned NOT NULL,
  `deleted` tinyint(1) unsigned DEFAULT '0',
  PRIMARY KEY (`guid`)
) ENGINE=MyISAM AUTO_INCREMENT=29377 DEFAULT CHARSET=utf8;

