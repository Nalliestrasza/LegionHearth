CREATE TABLE IF NOT EXISTS `creature_log` (
  `guid` bigint(20) unsigned NOT NULL,
  `spawnerAccountId` int(11) unsigned NOT NULL,
  `spawnerPlayerId` bigint(20) unsigned NOT NULL,
  `saved` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;