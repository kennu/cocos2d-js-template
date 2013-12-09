//
// MainScene class
//
var MainScene = function(){};

MainScene.prototype.onDidLoadFromCCB = function () {
	cc.log('MainScene loaded.');
};

// Create callback for button
MainScene.prototype.onPressSignInButton = function()
{
  var scene = cc.BuilderReader.loadAsScene("interface/GameScene.ccbi");
  cc.Director.getInstance().replaceScene(scene);
};
