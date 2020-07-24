CREATE TABLE IF NOT EXISTS `gameobject_log` (
  `guid` bigint(20) unsigned DEFAULT NULL,
  `spawnerAccountId` int(10) unsigned DEFAULT NULL,
  `spawnerPlayerId` bigint(20) unsigned DEFAULT NULL,
  `dupplicationGuid` int(20) DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
