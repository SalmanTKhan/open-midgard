add_library(ro_ui STATIC
    UiSkin.h
    UiSkin.cpp
    UIWindow.h
    UIWindow.cpp
    UiScale.h
    UiScale.cpp
    TextScale.h
    TextScale.cpp
    UILoginWnd.h
    UILoginWnd.cpp
    UISelectServerWnd.h
    UISelectServerWnd.cpp
    UISelectCharWnd.h
    UISelectCharWnd.cpp
    UIMakeCharWnd.h
    UIMakeCharWnd.cpp
    UINewChatWnd.h
    UINewChatWnd.cpp
    UIBasicInfoWnd.h
    UIBasicInfoWnd.cpp
    UIStatusWnd.h
    UIStatusWnd.cpp
    NpcDialogColoredText.h
    NpcDialogColoredText.cpp
    UISayDialogWnd.h
    UISayDialogWnd.cpp
    UINpcMenuWnd.h
    UINpcMenuWnd.cpp
    UIPlayerContextMenuWnd.h
    UIPlayerContextMenuWnd.cpp
    UIJoinPartyAcceptWnd.h
    UIJoinPartyAcceptWnd.cpp
    UIMessengerGroupWnd.h
    UIMessengerGroupWnd.cpp
    UIPartyOptionWnd.h
    UIPartyOptionWnd.cpp
    UINpcInputWnd.h
    UINpcInputWnd.cpp
    UIChooseSellBuyWnd.h
    UIChooseSellBuyWnd.cpp
    UINotifyLevelUpWnd.h
    UINotifyLevelUpWnd.cpp
    UIItemWnd.h
    UIItemWnd.cpp
    UIStorageWnd.h
    UIStorageWnd.cpp
    UIItemInfoWnd.h
    UIItemInfoWnd.cpp
    UIItemCollectionWnd.h
    UIItemCollectionWnd.cpp
    UIItemCompositionWnd.h
    UIItemCompositionWnd.cpp
    UIItemIdentifyWnd.h
    UIItemIdentifyWnd.cpp
    UIShortCutWnd.h
    UIShortCutWnd.cpp
    UIItemShopWnd.h
    UIItemShopWnd.cpp
    UIItemPurchaseWnd.h
    UIItemPurchaseWnd.cpp
    UIItemSellWnd.h
    UIItemSellWnd.cpp
    UIShopCommon.h
    UIMinimapWnd.h
    UIMinimapWnd.cpp
    UIEquipWnd.h
    UIEquipWnd.cpp
    UISkillDescribeWnd.h
    UISkillDescribeWnd.cpp
    UISkillListWnd.h
    UISkillListWnd.cpp
    UIChooseWnd.h
    UIChooseWnd.cpp
    UIEmotionWnd.h
    UIEmotionWnd.cpp
    UISelectCartWnd.h
    UISelectCartWnd.cpp
    UIVirtualKeyboardWnd.h
    UIVirtualKeyboardWnd.cpp
    UIControllerWnd.h
    UIControllerWnd.cpp
    UIOptionWnd.h
    UIOptionWnd.cpp
    UIFrameWnd.h
    UIFrameWnd.cpp
    UIWindowMgr.h
    UIWindowMgr.cpp
    UIPcGage.h
    UIPcGage.cpp
    UIRechargeGage.h
    UIRechargeGage.cpp
    UIWaitWnd.h
    UIWaitWnd.cpp
    UILoadingWnd.h
    UILoadingWnd.cpp
    UICartWnd.h
    UICartWnd.cpp
    UIGuildWnd.h
    UIGuildWnd.cpp
    UIMailBoxWnd.h
    UIMailBoxWnd.cpp
    UIMailReadWnd.h
    UIMailReadWnd.cpp
    UIMailSendWnd.h
    UIMailSendWnd.cpp
    UIPetInfoWnd.h
    UIPetInfoWnd.cpp
    UIEggListWnd.h
    UIEggListWnd.cpp
    UIHomunInfoWnd.h
    UIHomunInfoWnd.cpp
    UIMercInfoWnd.h
    UIMercInfoWnd.cpp
)

target_include_directories(ro_ui PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(ro_ui PUBLIC
    ro_core
    ro_item
    ro_session
    ro_skill
)

# UIControllerWnd visualizes SDL_GamepadButton state from the gamepad
# binding table. Link SDL3 headers privately so the UI can consume those
# types without forcing SDL on every downstream target.
if(RO_HAS_GAMEPAD)
    if(TARGET SDL3::SDL3)
        target_link_libraries(ro_ui PRIVATE SDL3::SDL3)
    elseif(TARGET SDL3::SDL3-static)
        target_link_libraries(ro_ui PRIVATE SDL3::SDL3-static)
    endif()
endif()

if(RO_ENABLE_QT6_UI)
    target_link_libraries(ro_ui PUBLIC
        Qt6::Core
        Qt6::Gui
    )
endif()
