#import "@preview/tidy:0.2.0"
#let report(
  title: none,
  subtitle: none,
  authors: (),
  body
) = {
  // Configuración general del documento
  set document(author: authors, title: title)
  set text(size: 11pt)
  set par(justify: true)
  
  // Estilo para bloques de código
  show raw.where(block: true): it => block(
    fill: rgb("#f5f5f5"),
    inset: 10pt,
    radius: 4pt,
    width: 100%,
    text(
      font: "Courier New",
      size: 9pt,
      it
    )
  )
  
  // Portada
 page(
    numbering: none,
    margin: 1in,
    [
      #set align(center)
      #v(2in)
      #text(24pt, weight: "bold")[#title]
      #v(0.5in)
      #text(18pt)[#subtitle]
      #v(0.5in)
      // Placeholder para el logo de la Universidad de Salamanca (mucho más grande)
      #image("usal-logo.png", width: 100%)
      #v(0.5in)
      #for author in authors [
        #text(14pt)[#author] \
      ]
      #v(2in)
    ]
  )
  
  // Configuración de encabezado y pie de página
  set page(
    margin: 1in,
    numbering: "1",
    header: [
      #set text(size: 9pt)
      #grid(
        columns: (1fr, 1fr),
        align(left)[#image("usal-logo-apaisado.png", width: 40%)],
        align(right)[
          #text("GENSYM G2") \
          #text(authors.join(", "))
        ]
      )
      #line(length: 100%)
    ],
    footer: context [
      #set text(size: 9pt)
      #align(center)[#counter(page).display("1")]
    ]
  )
  
  // Índice
  page[
    #set align(left)
    #text(16pt, weight: "bold")[Índice]
    #v(0.5in)
    #outline(title: none, indent: 2em)
  ]
  
  // Contenido principal
  set heading(numbering: "1.")
  show heading: it => [
    #set text(size: 14pt, weight: "bold")
    #it
  ]
  
  body
}

// Aplicar la plantilla
#show: report.with(
  title: "Sistemas Inteligentes",
  subtitle: "GENSYM G2",
  authors: ("Miguel Barranquero Díez","Victor Martín Fuentes", "David Pérez Velasco")
)

// Importar el contenido del documento principal
#include "documento.typ"

// Incluir la bibliografía desde el archivo .bib
//#bibliography("bibliografia.bib", title: "Bibliografía", style: "ieee")