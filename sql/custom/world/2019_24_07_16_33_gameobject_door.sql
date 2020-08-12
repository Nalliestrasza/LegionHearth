CREATE TABLE IF NOT EXISTS `gameobject_door` (
  `entry` bigint(11) NOT NULL DEFAULT '0',
  `id_item` bigint(11) DEFAULT NULL,
  PRIMARY KEY (`entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;