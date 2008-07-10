<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version="1.0">

<xsl:import href="nlm23html.xsl"/>

<!-- **************************************************************** -->
<xsl:template match="/">
  <!-- surround with a div because result must always be well-formed doc -->
  <div>
    <xsl:apply-templates select="article/front/article-meta"/>
  </div>
</xsl:template>

<!-- **************************************************************** -->
<xsl:template match="article-meta">
  <xsl:choose>
    <xsl:when test="abstract[@abstract-type='toc']">
      <xsl:apply-templates select="abstract[@abstract-type='toc'][1]"/>
    </xsl:when>
    <xsl:when test="abstract[not(@abstract-type)]">
      <xsl:apply-templates select="abstract[not(@abstract-type)][1]"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:apply-templates select="abstract[1]"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!-- **************************************************************** -->
<xsl:template match="abstract">
  <xsl:apply-templates/>
</xsl:template>

</xsl:stylesheet>
