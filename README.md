# StellarInput

**StellarInput** est un plugin modulaire et découplé pour **Unreal Engine 5** qui fournit une couche d'abstraction supérieure au-dessus d'**Enhanced Input**. Il permet le routage universel des contrôles, l'empilement/dépilement hiérarchique de contextes d'entrées (Context Stacking), et le dispatching d'actions via des **Gameplay Tags**, sans aucune dépendance circulaire entre les modules.

---

## Fonctionnalités Clés

1. **Zéro Dépendance Circulaire :**
   - Conçu comme un module fondamental dans l'écosystème `Stellar`.
   - Tout plugin (`StellarBuilder`, `StellarLocomotion`, `Jupiter`, etc.) peut inclure `"StellarInput"` dans son `.Build.cs` sans dépendance croisée.

2. **Pile de Contextes d'Entrées (Context Stacking) :**
   - Gestion dynamique des contextes d'entrées (`UInputMappingContext`) via un système de pile.
   - `PushContext` / `PushInputConfig` applique automatiquement le contexte avec sa priorité dédiée.
   - `PopContext` / `PopInputConfig` restaure instantanément et proprement la configuration de contrôle précédente.

3. **Routage par Gameplay Tags :**
   - Les actions sont identifiées par des tags sémantiques (ex: `InputTag.Action.Jump`, `InputTag.Builder.Place`, `InputTag.Vehicle.Throttle`).
   - Flexibilité totale pour remapper ou rediriger des contrôles dynamiquement.

4. **Interface Universelle de Réception (`IStellarInputReceiverInterface`) :**
   - N'importe quel composant (système de construction, arme, véhicule, drone) peut implémenter l'interface pour intercepter les entrées de manière découplée du Pawn ou Controller.

5. **Performance AAA :**
   - Composants purement événementiels, sans tick frame-by-frame (`bCanEverTick = false`).
   - Utilisation de structures légères, `TWeakObjectPtr` et pointeurs d'interfaces sûrs.

---

## Architecture

```
+-----------------------------------------------------------------------------------+
|                             UStellarInputSubsystem                                |
|              (LocalPlayerSubsystem — Pile Globale & Routage Universel)            |
+-----------------------------------------------------------------------------------+
          ▲                                  ▲                              ▲
          │ Push / Pop Context               │ Tagged Action Event          │ Device Check
+-------------------------+      +-------------------------+      +------------------+
| UStellarInputConfigAsset|      | IStellarInputReceiver   |      | Hardware Device  |
|  • MappingContext       |      | • ReceiveInputAction()  |      | • Keyboard/Mouse |
|  • Priority             |      | • CanReceiveInputTag()  |      | • Gamepad        |
|  • TaggedActions[]      |      +-------------------------+      | • Flight Stick   |
+-------------------------+                  ▲                    +------------------+
          ▲                                  │
          │                                  │
+----------------------------------------------------------+
|                  UStellarInputManager                    |
|      (Pawn / Controller — Wrapper EnhancedInput)         |
+----------------------------------------------------------+
```

---

## Structure des Classes

| Classe / Fichier | Description |
| :--- | :--- |
| `UStellarInputSubsystem` | Sous-système LocalPlayer pour l'accès global et la gestion centralisée des contextes. |
| `UStellarInputManager` | Gestionnaire local qui orchestre la liaison aux composants Enhanced Input. |
| `UStellarInputConfigAsset` | Data Asset définissant un mapping context, sa priorité et ses actions tagguées. |
| `IStellarInputReceiverInterface` | Interface permettant à n'importe quel objet de recevoir des événements d'entrée. |
| `UStellarInputFunctionLibrary` | Bibliothèque de fonctions Blueprint/C++ pour empiler et dépiler des contextes. |
| `FStellarTaggedInputAction` | Structure liant un `UInputAction` à un `FGameplayTag` et un `ETriggerEvent`. |

---

## Guide d'Utilisation Rapide

### 1. Définir un Data Asset de Contrôles
Créez un asset dérivé de `UStellarInputConfigAsset` dans l'éditeur :
- Définissez l'**Input Mapping Context** (`IMC_...`).
- Définissez la **Priorité** (ex: `10` pour un mode spécial, `0` pour le mode par défaut).
- Associez vos `InputAction` aux tags correspondants dans la liste `TaggedActions`.

### 2. Empiler un Contexte (ex: Activation d'un mode spécifique)
```cpp
#include "Libraries/StellarInputFunctionLibrary.h"

// Activation
FGuid InputHandle = UStellarInputFunctionLibrary::PushInputConfig(PlayerController, MyInputConfigAsset);

// Désactivation / Sortie
UStellarInputFunctionLibrary::PopInputConfig(PlayerController, InputHandle);
```

### 3. Recevoir des Actions via l'Interface
Implémentez `IStellarInputReceiverInterface` sur votre classe :
```cpp
virtual void ReceiveStellarInputAction_Implementation(
    const FGameplayTag& InActionTag,
    const FInputActionValue& InValue,
    ETriggerEvent InTriggerEvent) override
{
    if (InActionTag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("InputTag.Action.Primary"))))
    {
        // Traitement de l'action
    }
}
```

---

## Configuration Requise

- **Moteur :** Unreal Engine 5.3+ (Testé sous UE 5.7)
- **Plugin requis :** `EnhancedInput` (activé automatiquement)
- **Plateformes :** Win64, Linux, Mac, Consoles

---

## Auteur

- **Auteur :** bixboy
- **Licence :** Propriétaire / Tous droits réservés
