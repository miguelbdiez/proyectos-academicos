#import "@preview/tidy:0.2.0"
#let report(
  title: none,
  subtitle: none,
  authors: (),
  body
) = {
  // Configuración general del documento
  set document(author: authors, title: title)
  set text(font: "Times New Roman", size: 11pt)
  set par(justify: true)

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
      // Placeholder para el logo de la Universidad de Salamanca
      #image("usal-logo.png", width: 100%)
      #v(0.5in)
      #text(14pt)[#authors]

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
          #text("Detección de errores, defectos y fallos de software") \
          #text(if type(authors) == array { authors.join(", ") } else { authors })
        ]
      )
      #line(length: 100%)
    ],
    footer: [
      #set text(size: 9pt)
      #align(center)[#counter(page).display()]
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
  title: "Calidad y Auditoría",
  subtitle: "Investigación y aplicación de herramientas para pruebas \n unitarias en un lenguaje de programación (Python)",
  authors: "Miguel Barranquero Diez",
)

// Importar el contenido del documento principal
#include "entrega5.typ"

// Incluir la bibliografía desde el archivo .bib
#bibliography("bibliografia.bib", title: "Bibliografía", style: "ieee")