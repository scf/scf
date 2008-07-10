<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version="1.0">

<xsl:template match="*" mode="plaintext">
  <xsl:apply-templates mode="plaintext"/>
</xsl:template>

<xsl:template match="text()" mode="plaintext">
  <xsl:value-of select="."/>
</xsl:template>

<xsl:template match="footnote|superscript|remark" mode="plaintext">
  <!-- discard -->
</xsl:template>

</xsl:stylesheet>
