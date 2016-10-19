using UnityEngine;
using System.Collections;

public class Active : MonoBehaviour {
    public bool active = true;
    public void Deactivate()
    {
        active = false;
    }
}
