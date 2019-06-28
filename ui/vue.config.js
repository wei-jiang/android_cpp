module.exports = {
  productionSourceMap: false,
  outputDir : "../cordova/www",
  publicPath : "",
  runtimeCompiler: true,
  pluginOptions: {
    i18n: {
      locale: 'en',
      fallbackLocale: 'en',
      localeDir: 'locales',
      enableInSFC: true
    }
  }
}
