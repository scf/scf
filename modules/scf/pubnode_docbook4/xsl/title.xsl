<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version="1.0">

<xsl:import href="./docbook-xsl-1.73.2/html/docbook.xsl"/>
<xsl:import href="./plaintext.xsl"/>

<xsl:output method="text"/>

<xsl:template match="/">
  <xsl:apply-templates select="article/articleinfo/title" mode="plaintext"/>
</xsl:template>

</xsl:stylesheet>
