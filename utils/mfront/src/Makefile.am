AM_CPPFLAGS = -I$(top_srcdir)/utils/mfront/include    \
	      -I$(top_srcdir)/include                     \
	      @QT_CXXFLAGS@                               \
	      `@TFELCONFIG@ --includes --cppflags`

packagesdir = @prefix@/share/qemacs/packages
packages_LTLIBRARIES = libQEmacsMFrontImportWidgets.la

libQEmacsMFrontImportWidgets_la_SOURCES =              \
				MFMFilterProxyModel.cxx              \
				MFMDataBase.cxx                      \
				BehaviourDescription.cxx             \
				MaterialPropertiesSelectionPage.cxx  \
		         SelectMFMBehaviour.cxx               \
				SelectBehaviourPage.cxx              \
				ImportBehaviour.cxx                  \
				ImportMFMMaterialProperty.cxx        \
				ImportMFMBehaviour.cxx               \
				MaterialPropertySelectorWidget.cxx

libQEmacsMFrontImportWidgets_la_CPPFLAGS = $(AM_CPPFLAGS) -DQEmacsMFrontImportQWidgets_EXPORTS
libQEmacsMFrontImportWidgets_la_LIBADD   = -L@TFELHOME@/lib \
			         -lTFELMTest                 \
			         -lTFELMFront                \
			         -lTFELSystem

nodist_libQEmacsMFrontImportWidgets_la_SOURCES =                 \
				       moc_MFMFilterProxyModel.cxx             \
				       moc_SelectMFMBehaviour.cxx              \
				       moc_SelectBehaviourPage.cxx             \
				       moc_MaterialPropertiesSelectionPage.cxx \
				       moc_ImportMFMMaterialProperty.cxx       \
				       moc_ImportMFMBehaviour.cxx              \
				       moc_MaterialPropertySelectorWidget.cxx

moc_%.cxx : @top_srcdir@/utils/mfront/include/QEmacs/%.hxx
	@QT_MOC@ $< -o $@	

clean-local:
	$(RM) $(nodist_libQEmacsMFrontImportWidgets_la_SOURCES)

EXTRA_DIST=CMakeLists.txt
