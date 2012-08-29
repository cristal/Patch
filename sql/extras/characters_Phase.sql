SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `phase`
-- ----------------------------
DROP TABLE IF EXISTS `phase`;
CREATE TABLE `phase` (
  `guid` int(10) unsigned NOT NULL,
  `player_name` varchar(12) NOT NULL,
  `phase` int(11) NOT NULL,
  `phase_owned` int(11) NOT NULL,
  `get_phase` int(10) NOT NULL,
  `has_completed` tinyint(3) NOT NULL,
  PRIMARY KEY  (`guid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for `phase_members`
-- ----------------------------
DROP TABLE IF EXISTS `phase_members`;
CREATE TABLE `phase_members` (
  `guid` int(10) unsigned NOT NULL,
  `player_name` longtext NOT NULL,
  `can_build_in_phase` int(11) NOT NULL,
  `phase` int(11) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;