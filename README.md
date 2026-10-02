# Pong connecté sur ESP32

Un Pong à deux joueurs qui tourne sur **deux cartes ESP32**, chacune avec son écran OLED et son joystick. Les deux cartes communiquent directement entre elles en **ESP-NOW**, sans routeur ni serveur, et le terrain s'étend sur les deux écrans : la balle quitte l'écran d'un joueur pour arriver sur celui de l'autre.

Projet réalisé en binôme en 1ʳᵉ année de BUT Informatique (IUT Lyon 1, site de Bourg-en-Bresse), SAÉ 2.03, juin 2026.

## Démonstration

[![Vidéo de démonstration du Pong sur deux cartes ESP32](https://img.youtube.com/vi/keIEQiKue9o/hqdefault.jpg)](https://youtu.be/keIEQiKue9o)

Cliquer sur l'image pour voir la vidéo de démonstration (en anglais).

## Fonctionnalités

- **Mode multijoueur** sur deux cartes
  - la carte maître calcule la partie (balle, rebonds, vies) et envoie l'état du jeu toutes les 40 ms ;
  - la carte esclave renvoie la position de son joystick et affiche sa moitié du terrain ;
  - écran d'attente tant que l'autre joueur n'est pas là, et retour automatique à cet écran si le signal est perdu plus de 2 secondes.
- **Mode solo** contre une raquette automatique, avec 3 vies et un score.
- **Menus au joystick** : accueil, pause (appui long), fin de partie.
- La balle accélère au fil des échanges, avec un bip à chaque rebond.
- **Musique de fond** en multijoueur : la carte esclave joue le thème de Tetris sans bloquer le jeu.

## Matériel

Pour chaque joueur :

- 1 carte ESP32 au format Feather
- 1 écran OLED 128×64 SH1107 (I²C), avec son bouton C intégré
- 1 Grove Shield FeatherWing (Adafruit), sur lequel se branchent les modules sans soudure
- 1 joystick Grove, sur le port analogique A2/A3
- 1 haut-parleur Grove, sur le port A0

## Commandes

| Action | Geste |
|---|---|
| Naviguer dans les menus, déplacer la raquette | Joystick |
| Valider, lancer la balle | Clic du joystick |
| Mettre en pause (mode solo) | Clic maintenu une seconde |
| Revenir au menu | Bouton C de l'écran |

## Organisation du code

| Dossier | Rôle |
|---|---|
| `master/` | Carte maître : logique de jeu, envoi de l'état, affichage du joueur 1 |
| `slave/` | Carte esclave : envoi du joystick, affichage du joueur 2 |

Dans chaque dossier, `balle`, `barre` et `menu` sont des classes C++ séparées ; le fichier `.ino` contient la machine à états du jeu (menu, attente, solo, multi, pause, fin).

## Lancer le projet

1. Installer l'IDE Arduino avec le support ESP32 et les bibliothèques `Adafruit GFX` et `Adafruit SH110X`.
2. Remplacer les adresses MAC par celles de vos cartes : `adresseEsclave` dans `master/master.ino` et `adresseMaitre` dans `slave/slave.ino`.
3. Téléverser `master/` sur une carte et `slave/` sur l'autre.
4. Choisir « MULTI » sur les deux cartes : la partie démarre dès que la connexion est établie.

## Équipe

Projet mené à deux par **Yann Madry** et **Thomas Bonnefoy**.

## Ce que j'ai appris

- Faire communiquer deux microcontrôleurs en pair-à-pair : structures de messages, envoi périodique, fonction de réception.
- Garder un état de jeu cohérent entre deux machines en confiant tous les calculs à une seule (architecture maître / esclave).
- Jouer une mélodie sans bloquer la boucle de jeu, en s'appuyant sur `millis()` plutôt que sur `delay()`.
- Diagnostiquer une panne : nous avons longtemps cherché un bug dans le code alors que le problème venait d'une carte défectueuse.

## Pistes d'amélioration

- Écran « Paramètres » (réglage du son), aujourd'hui vide.
- Musique de fond sur la carte maître également.
