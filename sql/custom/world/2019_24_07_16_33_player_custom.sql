CREATE TABLE IF NOT EXISTS `player_custom` (
  `guid` bigint(10) unsigned NOT NULL,
  `displayId` int(10) unsigned DEFAULT '0',
  `scale` float DEFAULT '1',
  `skybox` int(255) unsigned DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
