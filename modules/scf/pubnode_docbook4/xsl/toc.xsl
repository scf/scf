<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version="1.0">

<xsl:import href="./docbook-xsl-1.73.2/xhtml/docbook.xsl"/>

<xsl:param name="section.autolabel" select="1"/>
<xsl:param name="generate.toc">
article   toc
sect1     toc
article/section   toc
</xsl:param>

<xsl:template match="/">
  <xsl:apply-templates select="article" mode="scf.toc"/>
</xsl:template>

<xsl:template match="article" mode="scf.toc">
  <xsl:variable name="toc.params">
    <xsl:call-template name="find.path.params">
      <xsl:with-param name="table" select="normalize-space($generate.toc)"/>
    </xsl:call-template>
  </xsl:variable>

  <xsl:call-template name="make.lots">
    <xsl:with-param name="toc.params" select="$toc.params"/>
    <xsl:with-param name="toc">
      <xsl:call-template name="component.toc">
        <xsl:with-param name="toc.title.p" select="contains($toc.params, 'title')"/>
      </xsl:call-template>
    </xsl:with-param>
  </xsl:call-template>
</xsl:template>

</xsl:stylesheet>
