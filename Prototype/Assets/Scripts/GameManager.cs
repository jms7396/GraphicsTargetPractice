using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;

public class GameManager : MonoBehaviour
{

    public GameObject Chain;
    List<GameObject> TargetList;
    GameObject gameCamera;
    GameObject scoreText;
    bool loaded = false;
    int activeTargets;
    bool rest = false;
    float restTime = 2.5f;
    int score = 0;

    // Use this for initialization
    void Start()
    {
        TargetList = new List<GameObject>();
        scoreText = GameObject.Find("Score");
        for (int i = 0; i < 4; i++)
        {
            TargetList.Add((GameObject)Instantiate(Chain, new Vector3(-3f + i * 2, 5f, 10f), Quaternion.identity));
        }
        activeTargets = 4;
        gameCamera = GameObject.FindGameObjectWithTag("MainCamera");
    }

    // Update is called once per frame
    void Update()
    {
        if (Input.GetMouseButtonDown(0))
            loaded = true;
        if (Input.GetMouseButtonUp(0))
        {
            if (loaded)
            {
                CheckShoot();
            }
            loaded = false;
        }
        if (Input.GetMouseButton(0))
        {
            Debug.DrawRay(gameCamera.transform.position, gameCamera.transform.forward * 5f);
        }
        if (rest)
        {
            restTime -= Time.deltaTime;
            if (restTime >= 1.2f && restTime <= 1.3f)
            {
                for (int i = 0; i < TargetList.Count; i++)
                {
                    Destroy(TargetList[i]);
                    TargetList.RemoveAt(i);
                }
            }
            if (restTime <= 0f)
            {
                rest = false;
                restTime = 2.5f;
                CreateNewTargets();
            }
        }

    }

    void CheckShoot()
    {
        RaycastHit hit;

        if (Physics.Raycast(gameCamera.transform.position, gameCamera.transform.forward, out hit) && (hit.transform.tag == "Target" || hit.transform.tag == "Chain"))
        {
            Active checkActive = (Active)hit.transform.gameObject.GetComponent(typeof(Active));
            if (checkActive.active)
            {
                if (hit.transform.tag == "Target")
                {
                    int index = 0;
                    for (int i = 0; i < TargetList.Count; i++)
                    {
                        if ((hit.transform.position.x == TargetList[i].transform.position.x) && (hit.transform.position.z == TargetList[i].transform.position.z))
                            index = i;
                    }
                    Destroy(TargetList[index]);
                    TargetList.RemoveAt(index);
                    activeTargets--;
                    UpdateScore(25);
                }

                if (hit.transform.tag == "Chain")
                {
                    hit.transform.gameObject.GetComponentInChildren<MeshRenderer>().enabled = false;
                    hit.transform.gameObject.GetComponentInChildren<SphereCollider>().enabled = false;
                    int index = 0;
                    for (int i = 0; i < TargetList.Count; i++)
                    {
                        if ((hit.transform.position.x == TargetList[i].transform.position.x) && (hit.transform.position.z == TargetList[i].transform.position.z))
                            index = i;
                    }

                    CreateTarget targetScript = (CreateTarget)TargetList[index].GetComponent(typeof(CreateTarget));
                    targetScript.DropObjectAtHeight(hit.transform.position.y);

                    Active deactivateScript = (Active)TargetList[index].GetComponent(typeof(Active));
                    deactivateScript.Deactivate();

                    activeTargets--;
                    UpdateScore(50);
                }

                if (activeTargets <= 0)
                {
                    rest = true;
                }
            }
        }
    }

    void CreateNewTargets()
    {
        for (int i = 0; i < 4; i++)
        {
            TargetList.Add((GameObject)Instantiate(Chain, new Vector3(-3f + i * 2, 5f, 10f), Quaternion.identity));
        }
        activeTargets = 4;
    }

    void UpdateScore(int amount)
    {
        score += amount;
        scoreText.GetComponent<Text>().text = "Score: " + score;
    }
}
