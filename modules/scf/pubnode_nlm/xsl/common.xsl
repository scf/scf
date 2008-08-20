<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet
  version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  exclude-result-prefixes="xsl">

<!-- **************************************************************** -->
<xsl:template match="sec" mode="num">
<xsl:apply-templates select="parent::sec" mode="num"/>
<xsl:value-of select="count(preceding-sibling::sec) + 1"/>
<xsl:text>.</xsl:text>
</xsl:template>


<!-- **************************************************************** -->
<xsl:template match="glossary" mode="gentitle">
  <xsl:call-template name="maketitle">
    <xsl:with-param name="title" select="'Glossary'"/>
    <xsl:with-param name="depth" select="1"/>
  </xsl:call-template>
</xsl:template>

 
<!-- **************************************************************** -->
<xsl:template match="ack" mode="gentitle">
  <xsl:call-template name="maketitle">
    <xsl:with-param name="title" select="'Acknowledgments'"/>
    <xsl:with-param name="depth" select="1"/>
  </xsl:call-template>
</xsl:template>
 

<!-- **************************************************************** -->
<xsl:template match="ref-list" mode="gentitle">
  <xsl:call-template name="maketitle">
    <xsl:with-param name="title" select="'References'"/>
    <xsl:with-param name="depth" select="1"/>
  </xsl:call-template>
</xsl:template>

 

<!-- **************************************************************** -->
<xsl:template match="*" mode="id">
  <xsl:param name="prefix" select="''"/>
  <xsl:value-of select="$prefix"/>
  <xsl:choose>
    <xsl:when test="@id">
      <xsl:value-of select="@id"/>
    </xsl:when>
    <xsl:otherwise>
      <!-- dont use generate-id() because ID must be predictable -->
      <xsl:text>gen_</xsl:text>
      <xsl:value-of select="local-name()"/>
      <xsl:text>_</xsl:text>
      <xsl:value-of select="count(preceding::*[local-name(current()) = local-name()])"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!-- **************************************************************** -->
<xsl:template match="*" mode="anchor">
  <xsl:param name="prefix" select="''"/>
  <xsl:variable name="id">
    <xsl:apply-templates select="." mode="id"/>
  </xsl:variable>
  <a name="{$id}"></a>
</xsl:template>


<!-- **************************************************************** -->
<!-- * From NCBI ViewNLM XSL **************************************** -->
<!-- **************************************************************** -->

	<!-- ============================================================= -->
	<!--  "capitalize" Capitalize a string                             -->
	<!-- ============================================================= -->

	<xsl:template name="capitalize">
		<xsl:param name="str"/>
		<xsl:value-of
			select="translate($str,
                          'abcdefghjiklmnopqrstuvwxyz',
                          'ABCDEFGHJIKLMNOPQRSTUVWXYZ')"
		/>
	</xsl:template>


</xsl:stylesheet>

