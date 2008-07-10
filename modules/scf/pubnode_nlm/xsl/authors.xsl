<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version="1.0">

<xsl:import href="./plaintext.xsl"/>

<xsl:output method="text"/>

<xsl:key name="element-by-id" match="*[@id]" use="@id"/>

<!-- **************************************************************** -->
<xsl:template match="/">
  <xsl:apply-templates select="article/front/article-meta"/>
</xsl:template>

<!-- **************************************************************** -->
<xsl:template match="article-meta">
  <xsl:apply-templates select="contrib-group/contrib"/>
</xsl:template>

<!-- **************************************************************** -->
<xsl:template match="contrib">
  <xsl:if test="position() > 1">
    <xsl:text>|</xsl:text>
  </xsl:if>
  <xsl:apply-templates select="name"/>
  <xsl:text>^</xsl:text>
  <!-- no email -->
  <xsl:text>^</xsl:text>
  <xsl:for-each select="xref[@ref-type='aff'][@rid]">
    <xsl:if test="position() > 1">
      <xsl:text>, </xsl:text>
    </xsl:if>
    <xsl:apply-templates select="key('element-by-id', @rid)"/>
  </xsl:for-each>
</xsl:template>

<!-- **************************************************************** -->
<xsl:template match="name">
  <xsl:choose>
    <xsl:when test="surname or given-names">
      <xsl:apply-templates select="surname" mode="plaintext"/>
      <xsl:text>^</xsl:text>
      <xsl:apply-templates select="given-names" mode="plaintext"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:apply-templates/>
      <xsl:text>^</xsl:text>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!-- **************************************************************** -->
<xsl:template match="aff">
  <xsl:apply-templates mode="plaintext"/>
</xsl:template>

<!-- **************************************************************** -->
<xsl:template match="aff/label" mode="plaintext">
  <!-- strip -->
</xsl:template>

<!-- **************************************************************** -->
<xsl:template match="/" mode="SAMPLE">
  <xsl:text>Jones^John^jones@gmail.org^Harvard Keyboard Manufacturers Assoc.</xsl:text>
</xsl:template>

</xsl:stylesheet>
