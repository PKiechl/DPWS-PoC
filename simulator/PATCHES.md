# Patch for Socket Memory Consumption

This patch was **not** written by me! Full credit for the code goes to [Tommaso Pecorella](https://gitlab.com/tommypec).
I have simply included the files here for the purpose of convenience, so users don't have to download the files themselves
form the corresponding git branch.

---
### What does the patch do? Why is it needed?

In order to enable the dynamic target switching behaviour that is inherent to
burst-wave DDoS attack patterns, modifications to the already existing `OnOffApplication`
were made to enable the setting of a new remote (target), resulting in the 
`OnOffRetargetApplication` that you can find in `contrib/distributed-pulse-wave-simulator/model`.

Due to having to support protocols (such as TCP) that have handshake procedures, 
the way the remote change is implemented requires
a socket exchange (i.e., closing the current socket and instantiating a new one with
the new remote address).

This can be problematic, due to sockets - up until now - not being, in layman's terms, "removed from memory" after
they are closed, thus continually consuming more and more memory as target switches are happening. 
For more context, refer to [this conversation](https://groups.google.com/g/ns-3-users/c/kfdMW6s9CjI)
in the Ns-3 community forum.

This ultimately led to one of the highly active contributors to Ns-3, [Tommaso Pecorella](https://gitlab.com/tommypec)
creating a [PullRequest](https://gitlab.com/nsnam/ns-3-dev/-/merge_requests/1515#e1c4a5ddc2cd9d4ebfd2dc426395b4a3370f2718)
that changes the way socket closing is handled, and in turn fixing the memory issue.

---
### Do I still need this patch with the Ns-3 Version I'm using?

At the time of writing this (June 2023), the [PullRequest](https://gitlab.com/nsnam/ns-3-dev/-/merge_requests/1515#e1c4a5ddc2cd9d4ebfd2dc426395b4a3370f2718) has not
yet been merged, but has apparently passed community scrutiny and has passed testing. The patch is flagged
for `Milestone-ns3.39`, thus if you are using a version Ns-3 before v3.39 then yes, you should use the patch files.

---
### How to use the patch files

I have included the patch files in the `socket memory patch` folder. All you have to do is copy the files and overwrite
the corresponding files in your Ns-3 installation (i.e., copy and paste into `src/internet/model`).


---

# Workaround for: NS_ASSERT failed, cond="m_ecmpRootExits.size() <= 1", msg="Assumed there is at most one exit from the root to this vertex"
This error occurs, when there are two equally valid routing options between two nodes in the global topology. What makes
this relevant for this project is the randomizes partial mesh topology implemented by the central network holding the
IXP nodes. This can very easily lead to a situation, where this assertion would fail.

This issue with the global routing regarding IPv4 is well known, and the assert is seen as perhaps
"[too restrictive](https://www.nsnam.org/bugzilla/show_bug.cgi?id=1965)", by [Tom Henderson](https://gitlab.com/tomhenderson),
one of the active contributors to Ns-3.

Users in [this thread](https://groups.google.com/g/ns-3-users/c/njclO2klIr0) also report that simply removing the assertion
is a valid approach and the simulation still produces the desired output.

I have therefore included a modified version of the file `src/internet/model/global-route-manager-impl.cc` in the
folder `overwrites`. Copy that file and overwrite the `src/internet/model/global-route-manager-impl.cc` in your Ns-3
installation.

Alternatively, if e.g., you are working with an already modified `global-route-manager-impl.cc` or are on a newer version
of Ns-3 that brought relevant changes to the file, then you may simply remove the ASSERT from the following function:
```
SPFVertex::NodeExit_t
SPFVertex::GetRootExitDirection() const
{
    NS_LOG_FUNCTION(this);
    // remove or uncomment the ASSERT below
    NS_ASSERT_MSG(m_ecmpRootExits.size() <= 1, "Assumed there is at most one exit from the root to this vertex");
    return GetRootExitDirection(0);
}
```
---