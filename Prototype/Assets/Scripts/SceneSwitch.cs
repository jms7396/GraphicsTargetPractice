using UnityEngine;
using System.Collections;
using UnityEngine.SceneManagement;

public class SceneSwitch : MonoBehaviour {

    public int scene;

    void OnMouseDown()
    {
        SceneManager.LoadScene(scene);
    }
}
