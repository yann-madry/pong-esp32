# Pong connecté sur ESP32

Un Pong à deux joueurs qui tourne sur **deux microcontrôleurs ESP32** reliés directement entre eux, sans routeur ni serveur. Le terrain s'étend sur les deux écrans : la balle quitte l'écran d'un joueur pour arriver sur celui de l'autre.

[![Vidéo de démonstration du Pong sur deux cartes ESP32](https://img.youtube.com/vi/keIEQiKue9o/hqdefault.jpg)](https://youtu.be/keIEQiKue9o)

*Cliquer sur l'image pour voir la démonstration (en anglais).*

## Contexte du projet

| | |
|---|---|
| Cadre | SAÉ 2.03, BUT Informatique 1ʳᵉ année, IUT Lyon 1 – site de Bourg-en-Bresse |
| Date et durée | Juin 2026, trois jours |
| Équipe | Binôme |
| Technologies | C++ (Arduino), ESP32, ESP-NOW, écran OLED SH1107 |

Le sujet : concevoir un jeu qui s'exécute simultanément sur plusieurs systèmes embarqués communicants.

## Présentation du projet

Chaque joueur dispose d'une carte ESP32 avec un écran OLED, un joystick et un haut-parleur. Les deux cartes forment un réseau pair-à-pair : l'une calcule la partie, l'autre affiche sa moitié du terrain et renvoie les mouvements de son joueur. Un mode solo contre une raquette automatique permet aussi de jouer avec une seule carte.

## Objectifs pédagogiques

Cette SAÉ mobilise principalement trois compétences du BUT Informatique :

- **Administrer des systèmes informatiques communicants** : identifier les composants matériels, configurer les cartes et les faire communiquer en réseau.
- **Réaliser un développement d'application** : programmer un jeu complet en C++ sur microcontrôleur, avec ses menus et ses modes.
- **Conduire un projet** : planifier le travail, versionner le code avec Git et présenter le résultat, en anglais.

## Travail réalisé

- Un jeu jouable à deux sur deux cartes, et en solo sur une seule.
- Une communication réseau sans fil qui garde la partie synchronisée en temps réel.
- Une vidéo de démonstration en anglais.

## Fonctionnalités

- **Mode multijoueur** sur deux cartes
  - la carte maître calcule la partie (balle, rebonds, vies) et envoie l'état du jeu toutes les 40 ms ;
  - la carte esclave renvoie la position de son joystick et affiche sa moitié du terrain ;
  - écran d'attente tant que l'autre joueur n'est pas connecté, et retour automatique à cet écran si le signal est perdu plus de 2 secondes.
- **Mode solo** contre une raquette automatique, avec 3 vies et un score.
- **Menus au joystick** : accueil, pause (appui long), fin de partie.
- La balle accélère au fil des échanges, avec un bip à chaque rebond.
- **Musique de fond** en multijoueur, jouée sans bloquer le jeu.

## Architecture du projet

```
.
├── README.md
├── LICENSE
└── code/
    ├── master/        Carte maître : logique de jeu, envoi de l'état, affichage du joueur 1
    │   ├── master.ino   Machine à états du jeu (menu, attente, solo, multi, pause, fin)
    │   ├── balle        Déplacement et rebonds de la balle
    │   ├── barre        Raquette
    │   └── menu         Menus navigables au joystick
    └── slave/         Carte esclave : envoi du joystick, affichage du joueur 2, musique
```

Toute la logique de la partie est calculée par une seule carte, la carte maître : les deux joueurs voient donc toujours la même partie, sans risque de désaccord entre les cartes.

## Organisation du travail

Projet mené en binôme par **Yann Madry** et **Thomas Bonnefoy**, avec un dépôt Git et un planning partagés.

### Ma contribution

J'ai pris en charge **la partie réseau, qui fait de ce Pong un jeu connecté** :

- la communication ESP-NOW entre les deux cartes : format des messages, envoi périodique, réception ;
- le mode multijoueur, réalisé avec Thomas : synchronisation maître / esclave, connexion et gestion de la perte de signal ;
- la musique de fond non bloquante sur la carte esclave ;
- les tests sur les cartes, en binôme.

## Documents

| Document | Contenu |
|---|---|
| [Vidéo de démonstration](https://youtu.be/keIEQiKue9o) | Présentation du jeu et de son fonctionnement, en anglais |

## Implémentation

### Points techniques

- **ESP-NOW** : protocole sans fil d'Espressif qui relie deux ESP32 directement, chacune étant identifiée par son adresse MAC. Pas de point d'accès, très peu de latence.
- **Deux messages** : `MessageMaitre` (mode de jeu, positions des raquettes et de la balle) et `MessageEsclave` (position du joystick, état de la carte), envoyés sous forme de structures C++.
- **Un terrain sur deux écrans** : le terrain virtuel fait 128 pixels de haut ; chaque carte n'affiche que sa moitié de 64 pixels.
- **Détection de déconnexion** : chaque carte mémorise l'heure du dernier message reçu et revient à l'écran d'attente après 2 secondes de silence.
- **Musique non bloquante** : les notes sont jouées en s'appuyant sur `millis()` plutôt que sur `delay()`, pour ne jamais figer la boucle de jeu.

### Matériel

Pour chaque joueur :

- 1 carte ESP32 au format Feather
- 1 écran OLED 128×64 SH1107 (I²C), avec son bouton C intégré
- 1 Grove Shield FeatherWing (Adafruit), sur lequel se branchent les modules sans soudure
- 1 joystick Grove, sur le port analogique A2/A3
- 1 haut-parleur Grove, sur le port A0

| Action | Commande |
|---|---|
| Naviguer dans les menus, déplacer la raquette | Joystick |
| Valider, lancer la balle | Clic du joystick |
| Mettre en pause (mode solo) | Clic maintenu une seconde |
| Revenir au menu | Bouton C de l'écran |

### Compiler et téléverser

1. Installer l'IDE Arduino avec le support ESP32 et les bibliothèques `Adafruit GFX` et `Adafruit SH110X`.
2. Remplacer les adresses MAC par celles de vos cartes : `adresseEsclave` dans `code/master/master.ino` et `adresseMaitre` dans `code/slave/slave.ino`.
3. Téléverser `code/master/` sur une carte et `code/slave/` sur l'autre.
4. Choisir « MULTI » sur les deux cartes : la partie démarre dès que la connexion est établie.

## Suite du projet

- Compléter l'écran « Paramètres », aujourd'hui vide, avec le réglage du son.
- Jouer aussi la musique sur la carte maître.
- Étendre la partie à plus de deux joueurs, ESP-NOW permettant de relier plusieurs cartes.

## Licence

Ce projet est sous licence [Creative Commons BY-NC 4.0](LICENSE) : vous pouvez le réutiliser et l'adapter en citant les auteurs, mais pas à des fins commerciales.
