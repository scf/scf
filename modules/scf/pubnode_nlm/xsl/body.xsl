<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet
  version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:xlink="http://www.w3.org/1999/xlink"
  exclude-result-prefixes="xsl xlink">

<xsl:import href="nlm23html.xsl"/>

<xsl:output method="xml" encoding="UTF-8"
    indent="no" doctype-public="-//W3C//DTD XHTML 1.0 Transitional//EN"
    doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd"/>

<xsl:param name="FILES_PREFIX" select="'[[FILES]]'"/>

<!-- **************************************************************** -->
<xsl:template match="/">
  <div class="pubnode nlm">
    <xsl:apply-templates select="article/*"/>
    <div id="backmatter">
      <xsl:apply-templates select="article/front/article-meta/permissions" mode="display"/>
      <xsl:apply-templates select="article/front/article-meta/copyright-statement" mode="display"/>
      <xsl:apply-templates select="article/front/article-meta/author-notes/corresp" mode="display"/>
    </div>
  </div>
</xsl:template>

<!-- **************************************************************** -->
<xsl:template match="article-title">
  <!-- strip -->
</xsl:template>

<!-- **************************************************************** -->
<xsl:template match="article-meta/permissions">
  <!-- do nothing till later -->
</xsl:template>


<!-- **************************************************************** -->
<xsl:template match="article-meta/copyright-statement">
  <!-- for compatibility in case copyright-statement is direct child of article-meta -->
</xsl:template>


<!-- **************************************************************** -->
<xsl:template match="article-meta/permissions" mode="display">
  <div class="permissions">
    <xsl:apply-templates select="*"/>
  </div>
</xsl:template>

<!-- **************************************************************** -->
<xsl:template match="article-meta/copyright-statement" mode="display">
  <div class="permissions">
    <xsl:call-template name="copyright-statement"/>
  </div>
</xsl:template>

<!-- **************************************************************** -->
<xsl:template match="graphic[@mime-subtype='tiff']">
  <xsl:variable name="href" select="@xlink:href"/>
  <a class="tiff_download">
    <xsl:attribute name="href">
      <xsl:apply-templates select="@xlink:href" mode="href"/>
    </xsl:attribute>
    <xsl:choose>
      <xsl:when test="key('alt-graphics', @id)">
        <xsl:apply-templates select="key('alt-graphics', @id)[1]" mode="altGraphic"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>TIFF version</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </a>
</xsl:template>

<!-- **************************************************************** -->
<xsl:template match="pub-date|history">
  <!-- strip for now -->
</xsl:template>


<!-- **************************************************************** -->
<xsl:template match="author-notes/corresp">
  <!-- do nothing till later -->
</xsl:template>


<!-- **************************************************************** -->
<xsl:template match="author-notes/corresp" mode="display">
  <div class="corresp">
    <xsl:apply-templates/>
  </div>
</xsl:template>


<!-- **************************************************************** -->
<xsl:template match="@*|*" mode="href">
  <xsl:variable name="raw" select="normalize-space(.)"/>
  <xsl:choose>
    <xsl:when test="starts-with($raw, 'http')">
      <xsl:value-of select="$raw"/>
    </xsl:when>
    <xsl:when test="starts-with($raw, '/')">
      <xsl:value-of select="concat($FILES_PREFIX, $raw)"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="concat($FILES_PREFIX, '/', $raw)"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>



</xsl:stylesheet>
