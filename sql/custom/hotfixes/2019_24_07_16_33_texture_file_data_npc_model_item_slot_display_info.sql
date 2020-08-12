DROP TABLE IF EXISTS `npc_model_item_slot_display_info`;
CREATE TABLE IF NOT EXISTS `npc_model_item_slot_display_info` (
  `ID` int(10) unsigned NOT NULL DEFAULT '0',
  `DisplayID` int(10) unsigned NOT NULL DEFAULT '0',
  `Slot` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `ExtendedDisplayID` int(10) unsigned NOT NULL DEFAULT '0',
  `VerifiedBuild` int(11) NOT NULL DEFAULT '35284',
  PRIMARY KEY (`ID`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;