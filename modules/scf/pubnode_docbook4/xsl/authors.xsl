<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version="1.0">

<xsl:import href="./docbook-xsl-1.73.2/xhtml/docbook.xsl"/>
<xsl:import href="./plaintext.xsl"/>

<xsl:output method="text"/>

<xsl:template match="/">
  <xsl:apply-templates select="article/articleinfo/author"/>
</xsl:template>

<xsl:template match="author">
  <xsl:if test="position() != 1">
    <xsl:text>|</xsl:text>
  </xsl:if>
  <xsl:apply-templates select="surname" mode="plaintext"/>
  <xsl:text>^</xsl:text>
  <xsl:apply-templates select="firstname|othername" mode="plaintext"/>
  <xsl:text>^</xsl:text>
  <xsl:apply-templates select="." mode="email"/>
  <xsl:text>^</xsl:text>
  <xsl:apply-templates select="affiliation"/>
</xsl:template>

<!-- expects the author element to contain somewhere a link like 
     <ulink url="mailto:slm@itsa.ucsf.edu">slm@itsa.ucsf.edu</ulink> -->
<xsl:template match="author" mode="email">
  <xsl:for-each select="descendant::ulink[starts-with(@url,'mailto:')]">
    <xsl:if test="position() != 1">
      <xsl:text>, </xsl:text>
    </xsl:if>
    <xsl:value-of select="normalize-space(substring-after(@url, 'mailto:'))"/>
  </xsl:for-each>
</xsl:template>

<xsl:template match="affiliation">
  <xsl:if test="position() != 1">
    <xsl:text>; </xsl:text>
  </xsl:if>
  <xsl:choose>
    <xsl:when test="shortaffil">
      <xsl:apply-templates select="shortaffil" mode="plaintext"/>
    </xsl:when>
    <xsl:when test="orgname">
      <xsl:apply-templates select="orgname" mode="plaintext"/>
      <xsl:if test="orgdiv">
        <xsl:text>, </xsl:text>
        <xsl:apply-templates select="orgdiv" mode="plaintext"/>
      </xsl:if>
    </xsl:when>
    <xsl:otherwise>
      <xsl:apply-templates select="address" mode="plaintext"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

</xsl:stylesheet>
