using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class FollowObject : MonoBehaviour
{

    public GameObject FollowingObject;
    //Vector3 initPosition;
    // Start is called before the first frame update
    void Start()
    {
        //initPosition = FollowingObject.transform.position;
    }

    // Update is called once per frame
    void Update()
    {
        if (FollowingObject)
        {
            this.transform.position = FollowingObject.transform.position;
        }
    }
}
