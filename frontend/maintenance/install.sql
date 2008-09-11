--
-- Table structure for table `bins`
--

DROP TABLE IF EXISTS `bins`;
CREATE TABLE `bins` (
  `id` int(11) NOT NULL auto_increment,
  `name` text,
  UNIQUE KEY `id` (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;
INSERT INTO `bins` VALUES (1, 'Default');

--
-- Table structure for table `files`
--

DROP TABLE IF EXISTS `files`;
CREATE TABLE `files` (
  `id` int(11) NOT NULL auto_increment,
  `fullpath` text,
  `bin_id` int(11) NOT NULL default '0',
  `sortorder` int(11) NOT NULL default '0',
  `active` tinyint(1) NOT NULL default '1',
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;
