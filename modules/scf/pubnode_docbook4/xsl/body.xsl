<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version="1.0">

<xsl:import href="./docbook-xsl-1.73.2/xhtml/docbook.xsl"/>

<xsl:param name="section.autolabel" select="1"/>
<xsl:param name="generate.toc" select="''"/>
<xsl:param name="footnote.number.symbols">*§</xsl:param>

<xsl:template match="/">
  <xsl:apply-templates select="*"/>
</xsl:template>

<!-- 
<xsl:template match="/*/*[descendant::bibliography]" priority="100">

</xsl:template>
-->

<xsl:template match="bibliography" mode="title.markup">
  <!-- suppress title in bibliography if containing section already has title. -->
</xsl:template>


<!-- suppress title -->
<xsl:template match="articleinfo/title" mode="titlepage.mode"/>

<!-- suppress footnotes -->
<xsl:template match="author/*/remark/footnote">
</xsl:template>

<xsl:template name="biblioentry.label">
<!-- suppress label -->
</xsl:template>

<!-- top-level section -->
<xsl:template match="/*/sect1|/*/section">
  <xsl:variable name="ln" select="local-name()"/>
  <xsl:variable name="num">
    <xsl:number select="preceding-sibling::*[local-name() = $ln]" format="1"/>
  </xsl:variable>
  <div id="scf_section_{$num}">
    <xsl:apply-imports/>
  </div>
</xsl:template>

</xsl:stylesheet>
