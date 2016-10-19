using UnityEngine;
using System.Collections;

public class CreateTarget : MonoBehaviour {

    public GameObject Target;
    public GameObject ChainChild;
    int childCount;
    public GameObject[] ObjectList;

	// Use this for initialization
	void Start () {
        childCount = Random.Range(3, 6);
        ObjectList = new GameObject[childCount + 1];
        for (int i = 1; i < childCount + 1; i++)
        {
            ObjectList[i -1] = (GameObject)Instantiate(ChainChild, new Vector3(transform.position.x, transform.position.y - (0.2f * i), transform.position.z), Quaternion.identity);
        }
        ObjectList[childCount] = (GameObject)Instantiate(Target, new Vector3(transform.position.x, transform.position.y - (0.2f * childCount) - 0.6f, transform.position.z), Quaternion.identity);
    }
	
    public void DropObjectAtHeight(float height)
    {
        int index = 0;
        for (int i = 0; i < 8; i++)
        {
            if (height == 5f - (0.2f * i))
            {
            }
        }

        for (int i = 1; i < childCount + 1; i++)
        {
            if (height == transform.position.y - (0.2f * i))
            {
                index = i + 1;
            }
        }

        for (int i = index; i < childCount + 1; i++)
        {
            ObjectList[i].GetComponent<Rigidbody>().useGravity = true;
        }
        Deactivate();
    }

    void Deactivate()
    {
        for (int i = 0; i < childCount + 1; i++)
        {
            Active deactivateScript = (Active)ObjectList[i].GetComponent(typeof(Active));
            deactivateScript.Deactivate();
        }
        
    }

    void OnDestroy()
    {
        for (int i = 0; i < childCount +1; i++)
        {
            Destroy(ObjectList[i]);
        }
    }
}
