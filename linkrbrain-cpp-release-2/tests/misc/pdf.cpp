#include "PDF/Document.hpp"
#include "Generators/Random.hpp"
#include "Generators/NumberName.hpp"

#include <iostream>
#include <fstream>


int main(int argc, char const *argv[]) {

    // set logger
    Logging::loggers.set_level(Logging::Level::Debug);
    Logging::add_output(Logging::Output::StandardError).set_color(true);

    // styles
    PDF::Style default_style = {
        .font_name = "Ubuntu-R",
        .font_size = 10,
        .line_height = 1.8,
        .paragraph_spacing = 0.5,
        .font_color = {0, 0, 0},
        .background_color = {.8, .9, 1},
        .margin_top = 1.0,
        .margin_bottom = 1.0,
        .margin_left = 2.0,
        .margin_right = 2.0,
    };
    PDF::Style title_style = {
        .font_name = "UbuntuMono-B",
        .font_size = 20,
        .line_height = 2.5,
        .paragraph_spacing = .5,
        .font_color = {0, .2, .5},
    };

    // create & save PDF
    PDF::Document pdf(default_style);
    pdf.load_font("var/fonts/ubuntu/UbuntuMono-B.ttf");
    pdf.load_font("var/fonts/ubuntu/Ubuntu-R.ttf");

    // first tries
    pdf.set_style(title_style);
    pdf.append("Titre 1");
    pdf.new_paragraph();
    pdf.set_style(default_style);
    pdf.append("- Hello!\n- Hello.");
    pdf.new_line();
    pdf.append("Ce texte est accentué : ");
    pdf.append("abcdÁéěňóšťíď. Celui-ci est ");
    pdf.append("rouge", {.font_color = {1,0,0}});
    pdf.append(", mais la suite ne l'est pas.");
    pdf.new_paragraph();
    pdf.append("Essayons avec un ");
    pdf.append("fond JAUNE", {.background_color = {1,1,0}});
    pdf.append(" à présent.");
    pdf.new_paragraph();

    // directly write paragraphs
    pdf.set_style({.background_color = PDF::None});
    pdf.append_paragraph("Write paragraphs", title_style);
    pdf.append_paragraph("György Cziffra est le puîné de trois enfants d'une famille de musiciens tziganes. György Cziffra père est cymbaliste et joue durant les années 1910 dans les restaurants et cabarets parisiens. Survient la Première Guerre mondiale. Le père est emprisonné en tant que citoyen d'un pays en guerre contre la France et la mère expulsée dans son pays d'origine. Elle vivra chichement avec ses deux filles dans une chambre de Budapest. Son mari, libéré de prison après la fin de la guerre, rejoint sa famille. Les retrouvailles se soldent par la naissance du petit György Cziffra le 5 novembre 1921.");
    pdf.append_paragraph("Dès son enfance, ce dernier montre un don particulier pour la musique. Son père lui donne ses premières leçons de piano, et à l'âge de quatre ans, il reproduit à l'oreille ce que joue sa sœur aînée. À cinq ans, il interprète des airs suggérés par le public d'un cirque itinérant dont il est, pendant quelques semaines seulement, la vedette. Il est, à neuf ans, le plus jeune élève jamais admis dans la prestigieuse Académie Franz Liszt de Budapest. Il y est formé par István Thomán et Ernő Dohnányi. À 13 ans, il finit l'opérette d'un autre compositeur en un temps record. Dès l'âge de seize ans il débute les tournées à travers l'Europe, notamment en Hongrie, Pays-Bas et Scandinavie.");
    pdf.append_paragraph("György Cziffra épouse Soleilka, une femme d'origine égyptienne, en 1941. Elle lui donnera un fils, également nommé György Cziffra, qui deviendra chef d'orchestre.");
    pdf.append_paragraph("La Deuxième Guerre mondiale contraint Cziffra à cesser d'étudier la musique. Il est envoyé combattre sur le front de l'Est avec l'Armée hongroise.La Hongrie est alors alliée de l'Allemagne. Il est fait prisonnier par des partisans soviétiques. Transféré quelques mois plus tard dans un camp de prisonniers, il est enrôlé dans la nouvelle armée hongroise qui se forme à la libération du territoire hongrois par l'Armée Rouge. Après avoir servi pendant plus d'un an comme instructeur, il est démobilisé et rejoint en 1946 sa femme et son fils qu'il n'avait pas revus depuis 1942.");
    pdf.append_paragraph("Il reprend l'étude du piano en 1947 auprès de György Ferenczy tout en gagnant sa vie en se produisant dans des bars de Budapest, en particulier avec son ami Elek Bacsik. Opposé au régime communiste hongrois, il est arrêté lors de sa tentative de traverser la frontière clandestinement avec sa famille. Il reste prisonnier politique de 1950 à 1953 1, condamné aux travaux forcés où il exécute la dure tâche de porteur de pierres. Il lui en restera des séquelles qui lui vaudront, à sa libération, de longs mois de rééducation et des douleurs persistantes aux articulations. D'où son fameux bracelet de cuir au poignet droit qu'il portera plusieurs années encore après son exil.");
    pdf.append_paragraph("Il est alors enfin reconnu comme un pianiste exceptionnel par le ministère hongrois des Affaires culturelles qui lui permet entre 1953 et 1956 d'accéder à une carrière d'interprète virtuose et de donner de nombreux concerts en Hongrie, sans pouvoir encore jouer à l'étranger. En 1955, il obtient le prix Franz Liszt de la virtuosité pianistique remis pour la première fois à un pianiste qui n'est pas lui-même compositeur. Le 22 octobre 1956, à l'occasion de la célébration de l'anniversaire de la Révolution d'Octobre, il donne au théâtre Erkél de Budapest une interprétation magistrale du 2e Concerto de Bartók, un concerto d'une difficulté extrême qu'il a appris en à peine six semaines grâce à un labeur acharné. Les spectateurs, enthousiastes, en sortent transportés : \"Ces quelque deux mille personnes, d'ordinaire si disciplinées, se ruèrent hors de la salle en scandant l'hymne national, arrachant sur leur passage dans les rues et boulevards avoisinants tout ce qui ne portait pas les couleurs nationales seules\". C'est alors le début de l'Insurrection de Budapest.");

    // try overflow
    pdf.new_page();
    pdf.append_paragraph("Writing without whitespaces", title_style);
    pdf.append_paragraph("LoremIpsumissimplydummytextoftheprintingandtypesettingindustry.LoremIpsumhasbeentheindustry'sstandarddummytexteversincethe1500s,whenanunknownprintertookagalleyoftypeandscrambledittomakeatypespecimenbook.Ithassurvivednotonlyfivecenturies,butalsotheleapintoelectronictypesetting,remainingessentiallyunchanged.Itwaspopularisedinthe1960swiththereleaseofLetrasetsheetscontainingLoremIpsumpassages,andmorerecentlywithdesktoppublishingsoftwarelikeAldusPageMakerincludingversionsofLoremIpsum.");

    // write many things until page overflows & new page are created
    pdf.new_page();
    for (size_t i=0; i<30; ++i) {
        pdf.append_paragraph("Fill the page", title_style);
        PDF::Style random_color_style = {.font_color = {
            Generators::Random::generate_number(0.f, 1.f),
            Generators::Random::generate_number(0.f, 1.f),
            Generators::Random::generate_number(0.f, 1.f),
        }};
        pdf.append_paragraph("Lorem Ipsum is simply dummy text of the printing and typesetting industry.\nLorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum.\nVoilà.", random_color_style);
    }

    // try showing a table
    pdf.new_page();
    Types::Table table({"Label", "x", "y", "z"});
    for (size_t i=0; i<100; ++i) {
        const size_t number = Generators::Random::generate_number(0, 1000);
        table.add_row(
            Generators::NumberName::get_english_name(number),
            Generators::Random::generate_number(-100, +100),
            Generators::Random::generate_number(-100, +100),
            Generators::Random::generate_number(-100, +100)
        );
    }
    pdf.append_table(table, {10, 2, 2, 2});

    // save file
    pdf.save("tmp/test.pdf");
    // pdf.save(std::cout, false);
    // std::ofstream output("/tmp/test2.pdf")
    // pdf.save(output);

    // the end!
    return 0;
}
