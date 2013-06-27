<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
   <xsl:output method="html"/>
   <xsl:template match="rgb">
      <html>
         <head>
            <title>X11 RGB Colour Table</title>
         </head>
         <body>
            <table border="1">
               <tr>
                  <th>Red</th>
                  <th>Green</th>
                  <th>Blue</th>
                  <th>Name</th>
                  <th>Preview</th>
               </tr>
               <xsl:for-each select="colour">
                  <tr>
                     <td><xsl:value-of select="red"/></td>
                     <td><xsl:value-of select="green"/></td>
                     <td><xsl:value-of select="blue"/></td>
                     <td><xsl:value-of select="name"/></td>
                     <td bgcolor="%htmlcode%" width="200"><xsl:value-of select="htmlcode"/></td>
                  </tr>
               </xsl:for-each>
            </table>
         </body>
      </html>
   </xsl:template>
</xsl:stylesheet>
