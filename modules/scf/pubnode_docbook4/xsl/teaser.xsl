<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version="1.0">

<xsl:import href="./docbook-xsl-1.73.2/xhtml/docbook.xsl"/>

<xsl:output method="xml" encoding="UTF-8" indent="no"/>

<xsl:template match="/">
  <xsl:apply-templates select="*/abstract"/>
</xsl:template>

<xsl:template match="abstract/title">
  <!-- remove abstract title -->
</xsl:template>

<xsl:template match="abstract/para">
  <xsl:apply-templates/>
  <!-- break up paras with at least a space -->
  <xsl:if test="following-sibling::*">
    <xsl:text> </xsl:text>
  </xsl:if>
</xsl:template>

<xsl:template match="emphasis[@role='bold']" priority="10">
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="emphasis">
  <em>
    <xsl:apply-templates/>
  </em>
</xsl:template>

<xsl:template match="*">
  <xsl:apply-templates/>
</xsl:template>

</xsl:stylesheet>
