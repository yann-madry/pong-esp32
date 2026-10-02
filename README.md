# Pong connecté sur ESP32

Un Pong à deux joueurs qui tourne sur **deux cartes ESP32**, chacune avec son écran OLED et son joystick. Les deux cartes communiquent directement entre elles en **ESP-NOW**, sans routeur ni serveur, et le terrain s'étend sur les deux écrans : la balle quitte l'écran d'un joueur pour arriver sur celui de l'autre.

Projet réalisé en binôme en 1ʳᵉ année de BUT Informatique (IUT Lyon 1, site de Bourg-en-Bresse), SAÉ 2.03, juin 2026.

## Fonctionnalités

- **Mode multijoueur** sur deux cartes
  - la carte maître calcule la partie (balle, rebonds, vies) et envoie l'état du jeu toutes les 40 ms ;
  - la carte esclave renvoie la position de son joystick et affiche sa moitié du terrain ;
  - écran d'attente tant que l'autre joueur n'est pas là, et retour automatique à cet écran si le signal est perdu plus de 2 secondes.
- **Mode solo** contre une raquette automatique, avec 3 vies et un score.
- **Menus au joystick** : accueil, pause (appui long), fin de partie.
- La balle accélère au fil des échanges, et un buzzer émet un bip à chaque rebond.

## Matériel

- 2 cartes Adafruit Feather ESP32
- 2 écrans OLED 128×64 (SH1107, FeatherWing)
- 2 joysticks analogiques
- 1 buzzer par carte

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
- Diagnostiquer une panne : nous avons longtemps cherché un bug dans le code alors que le problème venait d'une carte défectueuse.

## Pistes d'amélioration

- Écran « Paramètres » (réglage du son), aujourd'hui vide.
- Vraie musique à la place du simple bip.
