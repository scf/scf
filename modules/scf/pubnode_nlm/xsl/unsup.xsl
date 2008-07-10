<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version="1.0">

<!-- **************************************************************** -->
<xsl:template match="*">
  <span class="unsupported">
    <xsl:text>[[UNSUPPORTED:</xsl:text>
    <xsl:if test="parent::*">
      <xsl:value-of select="name(parent::*[1])"/>
      <xsl:text>/</xsl:text>
    </xsl:if>
    <xsl:value-of select="name()"/>
    <xsl:text>]]</xsl:text>
  </span>
</xsl:template>

</xsl:stylesheet>
