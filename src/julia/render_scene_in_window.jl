include("scene_driver.jl")
import .SceneDriver

if length(ARGS) != 1
  error("Must pass a path to a Entify scene as the first argument.")
end

SCENE_DIR = ARGS[1]

SceneDriver.RenderSceneInWindow(SCENE_DIR)
