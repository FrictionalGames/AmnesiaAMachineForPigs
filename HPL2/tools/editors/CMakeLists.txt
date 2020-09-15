cmake_minimum_required (VERSION 2.8)
project(Editors)

include_directories(
    ${HPL2_INCLUDE_DIR}
    ${DEP_INCLUDE_DIR}
)

link_directories(
    ${DEP_LIB_DIR}
)

### LevelEditor

file(GLOB_RECURSE leveleditor_sources
    BoxCreator.cpp
    DirectoryHandler.cpp
    EditorAction.cpp
    EditorActionArea.cpp
    EditorActionCompoundObject.cpp
    EditorActionDynamicEntity.cpp
    EditorActionEntity.cpp
    EditorActionHandler.cpp
    EditorActionMisc.cpp
    EditorActionSelection.cpp
    EditorAxisAlignedPlane.cpp
    EditorBaseClasses.cpp
    EditorClipPlane.cpp
    EditorEditMode.cpp
    EditorEditModeAreas.cpp
    EditorEditModeBillboards.cpp
    EditorEditModeCombine.cpp
    EditorEditModeDecals.cpp
    EditorEditModeEntities.cpp
    EditorEditModeFogAreas.cpp
    EditorEditModeLights.cpp
    EditorEditModeParticleSystems.cpp
    EditorEditModePrimitive.cpp
    EditorEditModeSelect.cpp
    EditorEditModeSelectTool.cpp
    EditorEditModeSelectToolRotate.cpp
    EditorEditModeSelectToolScale.cpp
    EditorEditModeSelectToolTranslate.cpp
    EditorEditModeSounds.cpp
    EditorEditModeStaticObjects.cpp
    EditorGrid.cpp
    EditorHelper.cpp
    EditorIndex.cpp
    EditorInput.cpp
    EditorSelection.cpp
    EditorThumbnailBuilder.cpp
    EditorUserClassDefinitionManager.cpp
    EditorVar.cpp
    EditorViewport.cpp
    EditorWindow.cpp
    EditorWindowAreas.cpp
    EditorWindowBillboards.cpp
    EditorWindowCombine.cpp
    EditorWindowDecals.cpp
    EditorWindowEditModeSidebar.cpp
    EditorWindowEntities.cpp
    EditorWindowEntityEditBox.cpp
    EditorWindowEntityEditBoxArea.cpp
    EditorWindowEntityEditBoxBillboard.cpp
    EditorWindowEntityEditBoxCompound.cpp
    EditorWindowEntityEditBoxDecal.cpp
    EditorWindowEntityEditBoxEntity.cpp
    EditorWindowEntityEditBoxFogArea.cpp
    EditorWindowEntityEditBoxGroup.cpp
    EditorWindowEntityEditBoxLight.cpp
    EditorWindowEntityEditBoxParticleSystem.cpp
    EditorWindowEntityEditBoxPrimitive.cpp
    EditorWindowEntityEditBoxSound.cpp
    EditorWindowEntityEditBoxStaticObject.cpp
    EditorWindowEntitySearch.cpp
    EditorWindowFactory.cpp
    EditorWindowFogAreas.cpp
    EditorWindowLights.cpp
    EditorWindowLoaderStatus.cpp
    EditorWindowLowerToolbar.cpp
    EditorWindowMaterialEditor.cpp
    EditorWindowObjectBrowser.cpp
    EditorWindowOptions.cpp
    EditorWindowParticleSystems.cpp
    EditorWindowPrimitives.cpp
    EditorWindowSelect.cpp
    EditorWindowSoundBrowser.cpp
    EditorWindowSounds.cpp
    EditorWindowStaticObjects.cpp
    EditorWindowTextureBrowser.cpp
    EditorWindowViewport.cpp
    EditorWorld.cpp
    EngineEntity.cpp
    EntityIcon.cpp
    EntityPicker.cpp
    EntityWrapper.cpp
    EntityWrapperArea.cpp
    EntityWrapperBillboard.cpp
    EntityWrapperCompoundObject.cpp
    EntityWrapperDecal.cpp
    EntityWrapperEntity.cpp
    EntityWrapperFogArea.cpp
    EntityWrapperLight.cpp
    EntityWrapperLightBox.cpp
    EntityWrapperLightPoint.cpp
    EntityWrapperLightSpot.cpp
    EntityWrapperParticleSystem.cpp
    EntityWrapperPrimitive.cpp
    EntityWrapperPrimitivePlane.cpp
    EntityWrapperSound.cpp
    EntityWrapperStaticObject.cpp
    SphereCreator.cpp
    StdAfx.cpp
    SurfacePicker.cpp

    LevelEditor.cpp
    LevelEditorActions.cpp
    LevelEditorMain.cpp
    LevelEditorStaticObjectCombo.cpp
    LevelEditorWindow.cpp
    LevelEditorWindowGroup.cpp
    LevelEditorWindowLevelSettings.cpp
    LevelEditorWorld.cpp
    )

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_SOURCE_DIR}/leveleditor/BuildID_LevelEditor_Linux.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/leveleditor/BuildID_LevelEditor.h
    COMMAND
        ${BUILDCOUNTER_BIN}
    ARGS
        LevelEditor ${CMAKE_CURRENT_SOURCE_DIR}/leveleditor ${CMAKE_CURRENT_SOURCE_DIR}/leveleditor
)

add_executable(LevelEditor EXCLUDE_FROM_ALL
    ${leveleditor_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/leveleditor/BuildID_LevelEditor_Linux.cpp)
target_link_libraries(LevelEditor HPL2)

### ModelEditor

file(GLOB_RECURSE modeleditor_sources
	BoxCreator.cpp
	DirectoryHandler.cpp
    EditorAction.cpp
    EditorActionCompoundObject.cpp
    EditorActionEntity.cpp
    EditorActionHandler.cpp
    EditorActionMisc.cpp
    EditorActionSelection.cpp
    EditorActionsBodies.cpp
    EditorActionsSubMesh.cpp
    EditorAxisAlignedPlane.cpp
    EditorBaseClasses.cpp
    EditorClipPlane.cpp
    EditorEditMode.cpp
    EditorEditModeBillboards.cpp
    EditorEditModeBodies.cpp
    EditorEditModeJoints.cpp
    EditorEditModeLights.cpp
    EditorEditModeParticleSystems.cpp
    EditorEditModeSelect.cpp
    EditorEditModeSelectTool.cpp
    EditorEditModeSelectToolRotate.cpp
    EditorEditModeSelectToolScale.cpp
    EditorEditModeSelectToolTranslate.cpp
    EditorEditModeSounds.cpp
    EditorGrid.cpp
    EditorHelper.cpp
    EditorIndex.cpp
    EditorInput.cpp
    EditorSelection.cpp
    EditorThumbnailBuilder.cpp
    EditorUserClassDefinitionManager.cpp
    EditorVar.cpp
    EditorViewport.cpp
    EditorWindow.cpp
    EditorWindowBillboards.cpp
    EditorWindowBodies.cpp
    EditorWindowEditModeSidebar.cpp
    EditorWindowEntities.cpp
    EditorWindowEntityEditBox.cpp
    EditorWindowEntityEditBoxBillboard.cpp
    EditorWindowEntityEditBoxBody.cpp
    EditorWindowEntityEditBoxBodyShape.cpp
    EditorWindowEntityEditBoxBone.cpp
    EditorWindowEntityEditBoxCompound.cpp
    EditorWindowEntityEditBoxGroup.cpp
    EditorWindowEntityEditBoxGroupShapes.cpp
    EditorWindowEntityEditBoxJoint.cpp
    EditorWindowEntityEditBoxLight.cpp
    EditorWindowEntityEditBoxParticleSystem.cpp
    EditorWindowEntityEditBoxSound.cpp
    EditorWindowEntityEditBoxSubMesh.cpp
    EditorWindowEntitySearch.cpp
    EditorWindowFactory.cpp
    EditorWindowJoints.cpp
    EditorWindowLights.cpp
    EditorWindowLoaderStatus.cpp
    EditorWindowLowerToolbar.cpp
    EditorWindowMaterialEditor.cpp
    EditorWindowObjectBrowser.cpp
    EditorWindowOptions.cpp
    EditorWindowParticleSystems.cpp
    EditorWindowSelect.cpp
    EditorWindowSoundBrowser.cpp
    EditorWindowSounds.cpp
    EditorWindowTextureBrowser.cpp
    EditorWindowViewport.cpp
    EditorWorld.cpp
    EngineEntity.cpp
    EntityIcon.cpp
    EntityPicker.cpp
    EntityWrapper.cpp
    EntityWrapperBillboard.cpp
    EntityWrapperBody.cpp
    EntityWrapperBodyShape.cpp
    EntityWrapperBone.cpp
    EntityWrapperCompoundObject.cpp
    EntityWrapperJoint.cpp
    EntityWrapperJointBall.cpp
    EntityWrapperJointHinge.cpp
    EntityWrapperJointScrew.cpp
    EntityWrapperJointSlider.cpp
    EntityWrapperLight.cpp
    EntityWrapperLightBox.cpp
    EntityWrapperLightPoint.cpp
    EntityWrapperLightSpot.cpp
    EntityWrapperParticleSystem.cpp
    EntityWrapperSound.cpp
    EntityWrapperSubMesh.cpp
    SphereCreator.cpp
    StdAfx.cpp
    SurfacePicker.cpp


    ModelEditor.cpp
    ModelEditorActions.cpp
    ModelEditorLowerToolbar.cpp
    ModelEditorMain.cpp
    ModelEditorWindowAnimations.cpp
    ModelEditorWindowOutline.cpp
    ModelEditorWindowPhysicsTest.cpp
    ModelEditorWindowUserSettings.cpp
    ModelEditorWorld.cpp
    )

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_SOURCE_DIR}/modeleditor/BuildID_ModelEditor_Linux.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/modeleditor/BuildID_ModelEditor.h
    COMMAND
        ${BUILDCOUNTER_BIN}
    ARGS
        ModelEditor ${CMAKE_CURRENT_SOURCE_DIR}/modeleditor ${CMAKE_CURRENT_SOURCE_DIR}/modeleditor
)

add_executable(ModelEditor EXCLUDE_FROM_ALL
    ${modeleditor_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/modeleditor/BuildID_ModelEditor_Linux.cpp)
target_link_libraries(ModelEditor HPL2)

### ParticleEditor

file(GLOB_RECURSE particleeditor_sources
    DirectoryHandler.cpp
    EditorAction.cpp
    EditorActionCompoundObject.cpp
    EditorActionEntity.cpp
    EditorActionHandler.cpp
    EditorActionMisc.cpp
    EditorActionSelection.cpp
    EditorAxisAlignedPlane.cpp
    EditorBaseClasses.cpp
    EditorClipPlane.cpp
    EditorEditMode.cpp
    EditorEditModeSelect.cpp
    EditorEditModeSelectTool.cpp
    EditorEditModeSelectToolRotate.cpp
    EditorEditModeSelectToolScale.cpp
    EditorEditModeSelectToolTranslate.cpp
    EditorGrid.cpp
    EditorHelper.cpp
    EditorIndex.cpp
    EditorInput.cpp
    EditorSelection.cpp
    EditorThumbnailBuilder.cpp
    EditorUserClassDefinitionManager.cpp
    EditorVar.cpp
    EditorViewport.cpp
    EditorWindow.cpp
    EditorWindowEditModeSidebar.cpp
    EditorWindowEntityEditBox.cpp
    EditorWindowEntityEditBoxCompound.cpp
    EditorWindowEntityEditBoxGroup.cpp
    EditorWindowEntitySearch.cpp
    EditorWindowFactory.cpp
    EditorWindowLoaderStatus.cpp
    EditorWindowLowerToolbar.cpp
    EditorWindowMaterialEditor.cpp
    EditorWindowOptions.cpp
    EditorWindowSelect.cpp
    EditorWindowSoundBrowser.cpp
    EditorWindowTextureBrowser.cpp
    EditorWindowViewport.cpp
    EditorWorld.cpp
    EngineEntity.cpp
    EntityIcon.cpp
    EntityPicker.cpp
    EntityWrapper.cpp
    EntityWrapperCompoundObject.cpp
    StdAfx.cpp
    SurfacePicker.cpp

    EntityWrapperParticleEmitter.cpp
    ParticleEditor.cpp
    ParticleEditorActions.cpp
    ParticleEditorMain.cpp
    ParticleEditorWindowEmitterParams.cpp
    ParticleEditorWindowEmitters.cpp
    ParticleEditorWorld.cpp
    )

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_SOURCE_DIR}/particleeditor/BuildID_ParticleEditor_Linux.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/particleeditor/BuildID_ParticleEditor.h
    COMMAND
        ${BUILDCOUNTER_BIN}
    ARGS
        ParticleEditor ${CMAKE_CURRENT_SOURCE_DIR}/particleeditor ${CMAKE_CURRENT_SOURCE_DIR}/particleeditor
)

add_executable(ParticleEditor EXCLUDE_FROM_ALL
    ${particleeditor_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/particleeditor/BuildID_ParticleEditor_Linux.cpp)
target_link_libraries(ParticleEditor HPL2)

### MaterialEditor

file(GLOB_RECURSE materialeditor_sources
    DirectoryHandler.cpp
    EditorAction.cpp
    EditorActionCompoundObject.cpp
    EditorActionEntity.cpp
    EditorActionHandler.cpp
    EditorActionMisc.cpp
    EditorActionSelection.cpp
    EditorAxisAlignedPlane.cpp
    EditorBaseClasses.cpp
    EditorClipPlane.cpp
    EditorEditMode.cpp
    EditorEditModeSelect.cpp
    EditorEditModeSelectTool.cpp
    EditorEditModeSelectToolRotate.cpp
    EditorEditModeSelectToolScale.cpp
    EditorEditModeSelectToolTranslate.cpp
    EditorGrid.cpp
    EditorHelper.cpp
    EditorIndex.cpp
    EditorInput.cpp
    EditorSelection.cpp
    EditorThumbnailBuilder.cpp
    EditorUserClassDefinitionManager.cpp
    EditorVar.cpp
    EditorViewport.cpp
    EditorWindow.cpp
    EditorWindowEditModeSidebar.cpp
    EditorWindowEntityEditBox.cpp
    EditorWindowEntityEditBoxCompound.cpp
    EditorWindowEntityEditBoxGroup.cpp
    EditorWindowEntitySearch.cpp
    EditorWindowFactory.cpp
    EditorWindowLoaderStatus.cpp
    EditorWindowLowerToolbar.cpp
    EditorWindowMaterialEditor.cpp
    EditorWindowOptions.cpp
    EditorWindowSelect.cpp
    EditorWindowSoundBrowser.cpp
    EditorWindowTextureBrowser.cpp
    EditorWindowViewport.cpp
    EditorWorld.cpp
    EngineEntity.cpp
    EntityIcon.cpp
    EntityPicker.cpp
    EntityWrapper.cpp
    EntityWrapperCompoundObject.cpp
    StdAfx.cpp
    SurfacePicker.cpp

    MaterialEditor.cpp
    MaterialEditorMain.cpp
    )

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_SOURCE_DIR}/materialeditor/BuildID_MaterialEditor_Linux.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/materialeditor/BuildID_MaterialEditor.h
    COMMAND
        ${BUILDCOUNTER_BIN}
    ARGS
        MaterialEditor ${CMAKE_CURRENT_SOURCE_DIR}/materialeditor ${CMAKE_CURRENT_SOURCE_DIR}/materialeditor
)

add_executable(MaterialEditor EXCLUDE_FROM_ALL
    ${materialeditor_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/materialeditor/BuildID_MaterialEditor_Linux.cpp)
target_link_libraries(MaterialEditor HPL2)

#install(TARGETS LevelEditor ModelEditor ParticleEditor MaterialEditor
#    RUNTIME DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/../../redist"
#    CONFIGURATIONS Debug)

#install(TARGETS LevelEditor ModelEditor ParticleEditor MaterialEditor
#    RUNTIME DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/../../Production"
#    CONFIGURATIONS Release RelWithDebInfo)

# vim: noet ts=4:

